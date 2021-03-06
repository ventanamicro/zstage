/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_STRING_H__
#define __ZSTAGE_STRING_H__

#include <zstage/types.h>

int strcmp(const char *a, const char *b);

int strncmp(const char *a, const char *b, size_t count);

size_t strlen(const char *str);

size_t strnlen(const char *str, size_t count);

char *strcpy(char *dest, const char *src);

char *strncpy(char *dest, const char *src, size_t count);

char *strchr(const char *s, int c);

char *strrchr(const char *s, int c);

void *memset(void *s, int c, size_t count);

void *memcpy(void *dest, const void *src, size_t count);

void *memmove(void *dest, const void *src, size_t count);

int memcmp(const void *s1, const void *s2, size_t count);

void *memchr(const void *s, int c, size_t count);

#endif
