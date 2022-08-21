/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_CONSOLE_H__
#define __ZSTAGE_CONSOLE_H__

#include <zstage/types.h>

struct zstage_console_device {
	/** Name of the console device */
	char name[32];

	/** Write a character to the console output */
	void (*console_putc)(char ch);
};

#define __printf(a, b) __attribute__((format(printf, a, b)))

bool isprintable(char ch);

void putc(char ch);

void puts(const char *str);

int __printf(2, 3) sprintf(char *out, const char *format, ...);

int __printf(3, 4) snprintf(char *out, u32 out_sz, const char *format, ...);

int __printf(1, 2) printf(const char *format, ...);

void __printf(1, 2) __noreturn panic(const char *format, ...);

const struct zstage_console_device *zstage_console_get_device(void);

void zstage_console_set_device(const struct zstage_console_device *dev);

int zstage_console_init(unsigned long boot_arg1);

#endif
