/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_PAYLOAD_BLOB_H__
#define __ZSTAGE_PAYLOAD_BLOB_H__

#ifndef __ASSEMBLER__

#include <zstage/types.h>

struct zstage_payload_blob {
	u64	src_addr;
	u64	dst_addr;
	u64	size;
	u64	reserved;
};

#endif

#endif
