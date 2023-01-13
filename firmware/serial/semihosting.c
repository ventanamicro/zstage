/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <zstage/console.h>
#include <zstage/error.h>
#include <zstage/serial/semihosting.h>
#include <zstage/string.h>

#define SYSOPEN     0x01
#define SYSWRITEC   0x03
#define SYSWRITE    0x05
#define SYSREAD     0x06
#define SYSREADC    0x07
#define SYSERRNO    0x13

static long semihosting_trap(int sysnum, void *addr)
{
	register int ret asm ("a0") = sysnum;
	register void *param0 asm ("a1") = addr;

	asm volatile (
		"	.align 4\n"
		"	.option push\n"
		"	.option norvc\n"

		"	slli zero, zero, 0x1f\n"
		"	ebreak\n"
		"	srai zero, zero, 7\n"

		"	.option pop\n"
		: "+r" (ret) : "r" (param0) : "memory");

	return ret;
}

static bool _semihosting_enabled = true;
static bool try_semihosting = true;

/*
 * Note: This function is intentionally noinline and 256 bytes aligned to
 * ensure that semihosting instruction sequence is within same 4K page.
 */
bool semihosting_test(void)
{
	register int ret asm ("a0") = SYSERRNO;
	register void *param0 asm ("a1") = NULL;
	unsigned long tmp = 0;

	if (!try_semihosting)
		return _semihosting_enabled;

	asm volatile (
		"	.align 4\n"
		"	.option push\n"
		"	.option norvc\n"

		"	j _semihost_test_vector_next\n"
		"	.align 4\n"
		"_semihost_test_vector:\n"
		"	csrr %[en], mepc\n"
		"	addi %[en], %[en], 4\n"
		"	csrw mepc, %[en]\n"
		"	add %[en], zero, zero\n"
		"	mret\n"
		"_semihost_test_vector_next:\n"

		"	la %[tmp], _semihost_test_vector\n"
		"	csrrw %[tmp], mtvec, %[tmp]\n"
		"	.align 4\n"
		"	slli zero, zero, 0x1f\n"
		"	ebreak\n"
		"	srai zero, zero, 7\n"
		"	csrw mtvec, %[tmp]\n"

		"	.option pop\n"
		: [tmp] "+r" (tmp), [en] "+r" (_semihosting_enabled),
		  [ret] "+r" (ret)
		: "r" (param0) : "memory");

	try_semihosting = false;
	return _semihosting_enabled;
}

static int semihosting_errno(void)
{
	long ret = semihosting_trap(SYSERRNO, NULL);

	if (ret > 0)
		return -ret;
	return -EIO;
}

static int semihosting_outfd = -ENODEV;

static long semihosting_open(const char *fname, enum semihosting_open_mode mode)
{
	long fd;
	struct semihosting_open_s {
		const char *fname;
		unsigned long mode;
		size_t len;
	} open;

	open.fname = fname;
	open.len = strlen(fname);
	open.mode = mode;

	/* Open the file on the host */
	fd = semihosting_trap(SYSOPEN, &open);
	if (fd == -1)
		return semihosting_errno();
	return fd;
}

/**
 * struct semihosting_rdwr_s - Arguments for read and write
 * @fd: A file descriptor returned from semihosting_open()
 * @memp: Pointer to a buffer of memory of at least @len bytes
 * @len: The number of bytes to read or write
 */
struct semihosting_rdwr_s {
	long fd;
	void *memp;
	size_t len;
};

static long semihosting_write(long fd, const void *memp, size_t len)
{
	long ret;
	struct semihosting_rdwr_s write;

	write.fd = fd;
	write.memp = (void *)memp;
	write.len = len;

	ret = semihosting_trap(SYSWRITE, &write);
	if (ret < 0)
		return semihosting_errno();
	return len - ret;
}

static void semihosting_putc(char ch)
{
	semihosting_trap(SYSWRITEC, &ch);
}

static void semihosting_puts(const char *str, unsigned long len)
{
	char ch;
	unsigned long i;

	if (semihosting_outfd < 0) {
		for (i = 0; i < len; i++) {
			ch = str[i];
			semihosting_trap(SYSWRITEC, &ch);
		}
	} else
		semihosting_write(semihosting_outfd, str, len);
}

static struct zstage_console_device semihost_console = {
	.name = "semihosting",
	.console_putc = semihosting_putc,
	.console_puts = semihosting_puts,
};

int semihosting_serial_init(void)
{
	if (!semihosting_test())
		return -ENODEV;
	semihosting_outfd = semihosting_open(":tt", MODE_WRITE);
	zstage_console_set_device(&semihost_console);
	return 0;
}
