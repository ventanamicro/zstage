/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_SERIAL_UART8250_H__
#define __ZSTAGE_SERIAL_UART8250_H__

#include <zstage/types.h>

int uart8250_init(unsigned long base, u32 in_freq,
		  u32 baudrate, u32 reg_shift, u32 reg_width);

#endif
