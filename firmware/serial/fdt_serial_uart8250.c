/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <libfdt.h>
#include <zstage/error.h>
#include <zstage/fdt/fdt_helper.h>
#include <zstage/serial/fdt_serial.h>
#include <zstage/serial/uart8250.h>

static int serial_uart8250_init(void *fdt, int nodeoff,
				const struct fdt_match *match)
{
	int rc;
	struct platform_uart_data uart;

	rc = fdt_parse_uart_node(fdt, nodeoff, &uart);
	if (rc)
		return rc;

	return uart8250_init(uart.addr, uart.freq, uart.baud,
			     uart.reg_shift, uart.reg_io_width);
}

static const struct fdt_match serial_uart8250_match[] = {
	{ .compatible = "ns16550" },
	{ .compatible = "ns16550a" },
	{ .compatible = "snps,dw-apb-uart" },
	{ },
};

struct fdt_serial fdt_serial_uart8250 = {
	.match_table = serial_uart8250_match,
	.init = serial_uart8250_init,
};
