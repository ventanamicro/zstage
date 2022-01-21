/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_MAIN_H__
#define __ZSTAGE_MAIN_H__

#include <zstage/types.h>

struct zstage_image_header;

/** Hang loop for failure situation */
void __noreturn zstage_hang(void);

/** The main function */
void zstage_main(unsigned long boot_arg1,
		 const struct zstage_image_header *hdr);

#endif
