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

struct platform_uart_data {
	unsigned long addr;
	unsigned long freq;
	unsigned long baud;
	unsigned long reg_shift;
	unsigned long reg_io_width;
};

#define DEFAULT_UART_FREQ		0
#define DEFAULT_UART_BAUD		115200
#define DEFAULT_UART_REG_SHIFT		0
#define DEFAULT_UART_REG_IO_WIDTH	1

static int fdt_parse_uart8250_node(void *fdt, int nodeoffset,
				   struct platform_uart_data *uart)
{
	int len, rc;
	const fdt32_t *val;
	uint64_t reg_addr, reg_size;

	if (nodeoffset < 0 || !uart || !fdt)
		return -ENODEV;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 0,
				    &reg_addr, &reg_size);
	if (rc < 0 || !reg_addr || !reg_size)
		return -ENODEV;
	uart->addr = reg_addr;

	/**
	 * UART address is mandaotry. clock-frequency and current-speed
	 * may not be present. Don't return error.
	 */
	val = (fdt32_t *)fdt_getprop(fdt, nodeoffset, "clock-frequency", &len);
	if (len > 0 && val)
		uart->freq = fdt32_to_cpu(*val);
	else
		uart->freq = DEFAULT_UART_FREQ;

	val = (fdt32_t *)fdt_getprop(fdt, nodeoffset, "current-speed", &len);
	if (len > 0 && val)
		uart->baud = fdt32_to_cpu(*val);
	else
		uart->baud = DEFAULT_UART_BAUD;

	val = (fdt32_t *)fdt_getprop(fdt, nodeoffset, "reg-shift", &len);
	if (len > 0 && val)
		uart->reg_shift = fdt32_to_cpu(*val);
	else
		uart->reg_shift = DEFAULT_UART_REG_SHIFT;

	val = (fdt32_t *)fdt_getprop(fdt, nodeoffset, "reg-io-width", &len);
	if (len > 0 && val)
		uart->reg_io_width = fdt32_to_cpu(*val);
	else
		uart->reg_io_width = DEFAULT_UART_REG_IO_WIDTH;

	return 0;
}

static int serial_uart8250_init(void *fdt, int nodeoff,
				const struct fdt_match *match)
{
	int rc;
	struct platform_uart_data uart;

	rc = fdt_parse_uart8250_node(fdt, nodeoff, &uart);
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
