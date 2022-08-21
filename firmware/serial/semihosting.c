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

static inline int semihosting_call(int call, void *arg0)
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
	zstage_console_set_device(&semihost_console);
	return 0;
}
