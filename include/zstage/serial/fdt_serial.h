/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_FDT_SERIAL_H__
#define __ZSTAGE_FDT_SERIAL_H__

#include <zstage/types.h>

struct fdt_serial {
	const struct fdt_match *match_table;
	int (*init)(void *fdt, int nodeoff, const struct fdt_match *match);
};

int fdt_serial_init(void *fdt);

#endif
