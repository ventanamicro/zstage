/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_IMAGE_HEADER_H__
#define __ZSTAGE_IMAGE_HEADER_H__

#define ZSTAGE_IMAGE_MAGIC			0x6774737a /* "zstg" */

#define ZSTAGE_IMAGE_MKVERSION(__major, __minor)\
	((__major) << 16 | (__minor))
#define ZSTAGE_IMAGE_VERSION			ZSTAGE_IMAGE_MKVERSION(1, 0)

#ifndef __ASSEMBLER__

#include <zstage/types.h>

struct zstage_image_header {
	u32	jump_insn;
	u32	magic;
	u32	version;
	u32	reserved1;
	u64	image_run_addr;
	u64	image_size;
	u64	next_load_addr;
	u64	next_run_addr;
	u64	next_fdt_addr;
	u64	next_size;
	u64	reserved2[8];
};

#endif

#endif
