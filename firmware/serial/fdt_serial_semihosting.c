/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <zstage/fdt/fdt_helper.h>
#include <zstage/serial/fdt_serial.h>
#include <zstage/serial/semihosting.h>

static const struct fdt_match serial_semihosting_match[] = {
	{ .compatible = "riscv,semihosting" },
	{ },
};

static int serial_semihosting_init(void *fdt, int nodeoff,
				   const struct fdt_match *match)
{
	return semihosting_serial_init();
}

struct fdt_serial fdt_serial_semihosting = {
	.match_table = serial_semihosting_match,
	.init = serial_semihosting_init
};
