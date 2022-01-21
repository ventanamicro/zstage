// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_FDT_HELPER_H__
#define __ZSTAGE_FDT_HELPER_H__

#include <zstage/types.h>

struct fdt_match {
	const char *compatible;
	void *data;
};

#define FDT_MAX_PHANDLE_ARGS 16
struct fdt_phandle_args {
	int node_offset;
	int args_count;
	u32 args[FDT_MAX_PHANDLE_ARGS];
};

const struct fdt_match *fdt_match_node(void *fdt, int nodeoff,
				       const struct fdt_match *match_table);

int fdt_find_match(void *fdt, int startoff,
		   const struct fdt_match *match_table,
		   const struct fdt_match **out_match);

int fdt_parse_phandle_with_args(void *fdt, int nodeoff,
				const char *prop, const char *cells_prop,
				int index, struct fdt_phandle_args *out_args);

int fdt_get_node_addr_size(void *fdt, int node, int index,
			   uint64_t *addr, uint64_t *size);

int fdt_parse_compat_addr(void *fdt, uint64_t *addr,
			  const char *compatible);

#endif
