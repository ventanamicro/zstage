// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <libfdt.h>
#include <zstage/error.h>
#include <zstage/fdt/fdt_helper.h>

const struct fdt_match *fdt_match_node(void *fdt, int nodeoff,
				       const struct fdt_match *match_table)
{
	int ret;

	if (!fdt || nodeoff < 0 || !match_table)
		return NULL;

	while (match_table->compatible) {
		ret = fdt_node_check_compatible(fdt, nodeoff,
						match_table->compatible);
		if (!ret)
			return match_table;
		match_table++;
	}

	return NULL;
}

int fdt_find_match(void *fdt, int startoff,
		   const struct fdt_match *match_table,
		   const struct fdt_match **out_match)
{
	int nodeoff;

	if (!fdt || !match_table)
		return -ENODEV;

	while (match_table->compatible) {
		nodeoff = fdt_node_offset_by_compatible(fdt, startoff,
						match_table->compatible);
		if (nodeoff >= 0) {
			if (out_match)
				*out_match = match_table;
			return nodeoff;
		}
		match_table++;
	}

	return -ENODEV;
}

int fdt_parse_phandle_with_args(void *fdt, int nodeoff,
				const char *prop, const char *cells_prop,
				int index, struct fdt_phandle_args *out_args)
{
	u32 i, pcells;
	int len, pnodeoff;
	const fdt32_t *list, *list_end, *val;

	if (!fdt || (nodeoff < 0) || !prop || !cells_prop || !out_args)
		return -EINVAL;

	list = fdt_getprop(fdt, nodeoff, prop, &len);
	if (!list)
		return -ENOENT;
	list_end = list + (len / sizeof(*list));

	while (list < list_end) {
		pnodeoff = fdt_node_offset_by_phandle(fdt,
						fdt32_to_cpu(*list));
		if (pnodeoff < 0)
			return pnodeoff;
		list++;

		val = fdt_getprop(fdt, pnodeoff, cells_prop, &len);
		if (!val)
			return -ENOENT;
		pcells = fdt32_to_cpu(*val);
		if (FDT_MAX_PHANDLE_ARGS < pcells)
			return -EINVAL;
		if (list + pcells > list_end)
			return -ENOENT;

		if (index > 0) {
			list += pcells;
			index--;
		} else {
			out_args->node_offset = pnodeoff;
			out_args->args_count = pcells;
			for (i = 0; i < pcells; i++)
				out_args->args[i] = fdt32_to_cpu(list[i]);
			return 0;
		}
	}

	return -ENOENT;
}

static int fdt_translate_address(void *fdt, uint64_t reg, int parent,
				 uint64_t *addr)
{
	int i, rlen;
	int cell_addr, cell_size;
	const fdt32_t *ranges;
	uint64_t offset = 0, caddr = 0, paddr = 0, rsize = 0;

	cell_addr = fdt_address_cells(fdt, parent);
	if (cell_addr < 1)
		return -ENODEV;

	cell_size = fdt_size_cells(fdt, parent);
	if (cell_size < 0)
		return -ENODEV;

	ranges = fdt_getprop(fdt, parent, "ranges", &rlen);
	if (ranges && rlen > 0) {
		for (i = 0; i < cell_addr; i++)
			caddr = (caddr << 32) | fdt32_to_cpu(*ranges++);
		for (i = 0; i < cell_addr; i++)
			paddr = (paddr << 32) | fdt32_to_cpu(*ranges++);
		for (i = 0; i < cell_size; i++)
			rsize = (rsize << 32) | fdt32_to_cpu(*ranges++);
		if (reg < caddr || caddr >= (reg + rsize ))
			return -ENODEV;
		offset = reg - caddr;
		*addr = paddr + offset;
	} else {
		/* No translation required */
		*addr = reg;
	}

	return 0;
}

int fdt_get_node_addr_size(void *fdt, int node, int index,
			   uint64_t *addr, uint64_t *size)
{
	int parent, len, i, rc;
	int cell_addr, cell_size;
	const fdt32_t *prop_addr, *prop_size;
	uint64_t temp = 0;

	if (!fdt || node < 0 || index < 0)
		return -EINVAL;

	parent = fdt_parent_offset(fdt, node);
	if (parent < 0)
		return parent;
	cell_addr = fdt_address_cells(fdt, parent);
	if (cell_addr < 1)
		return -ENODEV;

	cell_size = fdt_size_cells(fdt, parent);
	if (cell_size < 0)
		return -ENODEV;

	prop_addr = fdt_getprop(fdt, node, "reg", &len);
	if (!prop_addr)
		return -ENODEV;

	if ((len / sizeof(u32)) <= (index * (cell_addr + cell_size)))
		return -EINVAL;

	prop_addr = prop_addr + (index * (cell_addr + cell_size));
	prop_size = prop_addr + cell_addr;

	if (addr) {
		for (i = 0; i < cell_addr; i++)
			temp = (temp << 32) | fdt32_to_cpu(*prop_addr++);
		do {
			if (parent < 0)
				break;
			rc  = fdt_translate_address(fdt, temp, parent, addr);
			if (rc)
				break;
			parent = fdt_parent_offset(fdt, parent);
			temp = *addr;
		} while (1);
	}
	temp = 0;

	if (size) {
		for (i = 0; i < cell_size; i++)
			temp = (temp << 32) | fdt32_to_cpu(*prop_size++);
		*size = temp;
	}

	return 0;
}

int fdt_parse_compat_addr(void *fdt, uint64_t *addr,
			  const char *compatible)
{
	int nodeoffset, rc;

	nodeoffset = fdt_node_offset_by_compatible(fdt, -1, compatible);
	if (nodeoffset < 0)
		return nodeoffset;

	rc = fdt_get_node_addr_size(fdt, nodeoffset, 0, addr, NULL);
	if (rc < 0 || !addr)
		return -ENODEV;

	return 0;
}

#define DEFAULT_UART_FREQ		0
#define DEFAULT_UART_BAUD		115200
#define DEFAULT_UART_REG_SHIFT		0
#define DEFAULT_UART_REG_IO_WIDTH	1

int fdt_parse_uart_node(void *fdt, int nodeoffset,
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
