/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <zstage/console.h>
#include <zstage/error.h>
#include <zstage/serial/semihosting.h>

#define SEMIHOSTING_SYS_WRITEC		0x03
#define SEMIHOSTING_SYS_WRITE0		0x04
#define SEMIHOSTING_SYS_READC		0x07
#define SEMIHOSTING_SYS_SYSERRNO	0x13

/*
 * Note: This function is intentionally noinline and 256 bytes aligned to
 * ensure that semihosting instruction sequence is within same 4K page.
 */
static int __noinline __aligned(256) semihosting_call(int call, void *arg0)
{
	register int ret asm ("a0") = call;
	register void *param0 asm ("a1") = arg0;

	asm volatile (
		"\t.option push\n"
		"\t.option norvc\n"
		"\tslli zero, zero, 0x1f\n"
		"\tebreak\n"
		"\tsrai zero, zero, 7 \n"
		"\t.option pop \n"
		: "+r" (ret) : "r" (param0) : "memory");

	return ret;
}

static bool semihosting_en = true;
static bool try_semihosting = true;

/*
 * Note: This function is intentionally noinline and 256 bytes aligned to
 * ensure that semihosting instruction sequence is within same 4K page.
 */
bool __aligned(256) semihosting_test(void)
{
	unsigned long tmp = 0;
	register int ret asm ("a0") = SEMIHOSTING_SYS_SYSERRNO;
	register void *param0 asm ("a1") = NULL;

	if (!try_semihosting)
		return semihosting_en;

	asm volatile (
		"\t.option push\n"
		"\t.option norvc\n"

		"\tj _semihost_test_vector_next\n"
		"\t.align 4\n"
		"\t_semihost_test_vector:\n"
		"\t\tcsrr %[en], mepc\n"
		"\t\taddi %[en], %[en], 4\n"
		"\t\tcsrw mepc, %[en]\n"
		"\t\tadd %[en], zero, zero\n"
		"\t\tmret\n"
		"\t_semihost_test_vector_next:\n"

		"\tla %[tmp], _semihost_test_vector\n"
		"\tcsrrw %[tmp], mtvec, %[tmp]\n"
		"\tslli zero, zero, 0x1f\n"
		"\tebreak\n"
		"\tsrai zero, zero, 7 \n"
		"\tcsrw mtvec, %[tmp]\n"

		"\t.option pop \n"
		: [tmp] "+r" (tmp), [en] "+r" (semihosting_en),
		  [ret] "+r" (ret)
		: "r" (param0) : "memory");

	try_semihosting = false;
	return semihosting_en;
}

static void semihosting_putc(char ch)
{
	semihosting_call(SEMIHOSTING_SYS_WRITEC, &ch);
}

static struct zstage_console_device semihost_console = {
	.name = "semihosting",
	.console_putc = semihosting_putc,
};

int semihosting_serial_init(void)
{
	if (!semihosting_test())
		return -ENODEV;
	zstage_console_set_device(&semihost_console);
	return 0;
}
