/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_PLATFORM_H__
#define __ZSTAGE_PLATFORM_H__

struct zstage_image_header;

/** Early initialization of platform support */
int platform_early_init(unsigned long boot_arg1);

/** Initialize standard I/O console */
int platform_stdio_init(unsigned long boot_arg1);

/** Perpare (or setup) platform FDT at given address */
int platform_prepare_fdt(unsigned long boot_arg1,
			 const struct zstage_image_header *hdr);

/** Final initialization of platform support */
int platform_final_init(unsigned long boot_arg1);

#endif
