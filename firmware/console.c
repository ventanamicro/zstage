/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <zstage/console.h>
#include <zstage/main.h>
#include <zstage/platform.h>
#include <zstage/string.h>

static const struct zstage_console_device *console_dev = NULL;

bool isprintable(char c)
{
	if (((31 < c) && (c < 127)) || (c == '\f') || (c == '\r') ||
	    (c == '\n') || (c == '\t')) {
		return TRUE;
	}
	return FALSE;
}

void putc(char ch)
{
	if (console_dev && console_dev->console_putc) {
		if (ch == '\n')
			console_dev->console_putc('\r');
		console_dev->console_putc(ch);
	}
}

void nputs(const char *str, unsigned long len)
{
	unsigned long i;

	if (console_dev && console_dev->console_puts) {
		console_dev->console_puts(str, len);
	} else {
		for (i = 0; i < len; i++)
			putc(str[i]);
	}
}

void puts(const char *str)
{
	nputs(str, strlen(str));
}

#define PAD_RIGHT 1
#define PAD_ZERO 2
#define PAD_ALTERNATE 4
#define PRINT_BUF_LEN 64
#define PRINT_TBUF_MAX 128

#define va_start(v, l) __builtin_va_start((v), l)
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
typedef __builtin_va_list va_list;

static void printc(char **out, u32 *out_len, char ch)
{
	if (!out) {
		putc(ch);
		return;
	}

	/*
	 * The *printf entry point functions have enforced that (*out) can
	 * only be null when out_len is non-null and its value is zero.
	 */
	if (!out_len || *out_len > 1) {
		*(*out)++ = ch;
		**out = '\0';
	}

	if (out_len && *out_len > 0)
		--(*out_len);
}

static int prints(char **out, u32 *out_len, const char *string, int width,
		  int flags)
{
	int pc	     = 0;
	char padchar = ' ';

	if (width > 0) {
		int len = 0;
		const char *ptr;
		for (ptr = string; *ptr; ++ptr)
			++len;
		if (len >= width)
			width = 0;
		else
			width -= len;
		if (flags & PAD_ZERO)
			padchar = '0';
	}
	if (!(flags & PAD_RIGHT)) {
		for (; width > 0; --width) {
			printc(out, out_len, padchar);
			++pc;
		}
	}
	for (; *string; ++string) {
		printc(out, out_len, *string);
		++pc;
	}
	for (; width > 0; --width) {
		printc(out, out_len, padchar);
		++pc;
	}

	return pc;
}

static int printi(char **out, u32 *out_len, long long i, int b, int sg,
		  int width, int flags, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	char *s;
	int neg = 0, pc = 0;
	u64 t;
	unsigned long long u = i;

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u   = -i;
	}

	s  = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';

	if (!u) {
		*--s = '0';
	} else {
		while (u) {
			t = u % b;
			u = u / b;
			if (t >= 10)
				t += letbase - '0' - 10;
			*--s = t + '0';
		}
	}

	if (flags & PAD_ALTERNATE) {
		if ((b == 16) && (letbase == 'A')) {
			*--s = 'X';
		} else if ((b == 16) && (letbase == 'a')) {
			*--s = 'x';
		}
		*--s = '0';
	}

	if (neg) {
		if (width && (flags & PAD_ZERO)) {
			printc(out, out_len, '-');
			++pc;
			--width;
		} else {
			*--s = '-';
		}
	}

	return pc + prints(out, out_len, s, width, flags);
}

static int print(char **out, u32 *out_len, const char *format, va_list args)
{
	u32 tbuf_len;
	int width, flags, acnt = 0, pc = 0;
	char scr[2], *tout, tbuf[PRINT_TBUF_MAX];
	bool use_tbuf = (!out) ? true : false;
	unsigned long long tmp;

	if (use_tbuf) {
		tbuf_len = PRINT_TBUF_MAX;
		tout = tbuf;
		out = &tout;
		out_len = &tbuf_len;
	}

	for (; *format != 0; ++format) {
		if (use_tbuf && !tbuf_len) {
			nputs(tbuf, PRINT_TBUF_MAX);
			tbuf_len = PRINT_TBUF_MAX;
			tout = tbuf;
		}

		if (*format == '%') {
			++format;
			width = flags = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto out;
			/* Get flags */
			if (*format == '-') {
				++format;
				flags = PAD_RIGHT;
			}
			if (*format == '#') {
				++format;
				flags |= PAD_ALTERNATE;
			}
			while (*format == '0') {
				++format;
				flags |= PAD_ZERO;
			}
			/* Get width */
			for (; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if (*format == 's') {
				char *s = va_arg(args, char *);
				acnt += sizeof(char *);
				pc += prints(out, out_len, s ? s : "(null)",
					     width, flags);
				continue;
			}
			if ((*format == 'd') || (*format == 'i')) {
				pc += printi(out, out_len, va_arg(args, int),
					     10, 1, width, flags, '0');
				acnt += sizeof(int);
				continue;
			}
			if (*format == 'x') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned int), 16, 0,
					     width, flags, 'a');
				acnt += sizeof(unsigned int);
				continue;
			}
			if (*format == 'X') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned int), 16, 0,
					     width, flags, 'A');
				acnt += sizeof(unsigned int);
				continue;
			}
			if (*format == 'u') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned int), 10, 0,
					     width, flags, 'a');
				acnt += sizeof(unsigned int);
				continue;
			}
			if (*format == 'p') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned long), 16, 0,
					     width, flags, 'a');
				acnt += sizeof(unsigned long);
				continue;
			}
			if (*format == 'P') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned long), 16, 0,
					     width, flags, 'A');
				acnt += sizeof(unsigned long);
				continue;
			}
			if (*format == 'l' && *(format + 1) == 'l') {
				while (acnt &
				       (sizeof(unsigned long long) - 1)) {
					va_arg(args, int);
					acnt += sizeof(int);
				}
				if (sizeof(unsigned long long) ==
				    sizeof(unsigned long)) {
					tmp = va_arg(args, unsigned long long);
					acnt += sizeof(unsigned long long);
				} else {
					((unsigned long *)&tmp)[0] =
						va_arg(args, unsigned long);
					((unsigned long *)&tmp)[1] =
						va_arg(args, unsigned long);
					acnt += 2 * sizeof(unsigned long);
				}
				if (*(format + 2) == 'u') {
					format += 2;
					pc += printi(out, out_len, tmp, 10, 0,
						     width, flags, 'a');
				} else if (*(format + 2) == 'x') {
					format += 2;
					pc += printi(out, out_len, tmp, 16, 0,
						     width, flags, 'a');
				} else if (*(format + 2) == 'X') {
					format += 2;
					pc += printi(out, out_len, tmp, 16, 0,
						     width, flags, 'A');
				} else {
					format += 1;
					pc += printi(out, out_len, tmp, 10, 1,
						     width, flags, '0');
				}
				continue;
			} else if (*format == 'l') {
				if (*(format + 1) == 'u') {
					format += 1;
					pc += printi(
						out, out_len,
						va_arg(args, unsigned long), 10,
						0, width, flags, 'a');
				} else if (*(format + 1) == 'x') {
					format += 1;
					pc += printi(
						out, out_len,
						va_arg(args, unsigned long), 16,
						0, width, flags, 'a');
					acnt += sizeof(unsigned long);
				} else if (*(format + 1) == 'X') {
					format += 1;
					pc += printi(
						out, out_len,
						va_arg(args, unsigned long), 16,
						0, width, flags, 'A');
					acnt += sizeof(unsigned long);
				} else {
					pc += printi(out, out_len,
						     va_arg(args, long), 10, 1,
						     width, flags, '0');
					acnt += sizeof(long);
				}
			}
			if (*format == 'c') {
				/* char are converted to int then pushed on the stack */
				scr[0] = va_arg(args, int);
				scr[1] = '\0';
				pc += prints(out, out_len, scr, width, flags);
				acnt += sizeof(int);
				continue;
			}
		} else {
		out:
			printc(out, out_len, *format);
			++pc;
		}
	}

	if (use_tbuf && tbuf_len < PRINT_TBUF_MAX)
		nputs(tbuf, PRINT_TBUF_MAX - tbuf_len);

	return pc;
}

int sprintf(char *out, const char *format, ...)
{
	va_list args;
	int retval;

	va_start(args, format);
	retval = print(&out, NULL, format, args);
	va_end(args);

	return retval;
}

int snprintf(char *out, u32 out_sz, const char *format, ...)
{
	va_list args;
	int retval;

	va_start(args, format);
	retval = print(&out, &out_sz, format, args);
	va_end(args);

	return retval;
}

int printf(const char *format, ...)
{
	va_list args;
	int retval;

	va_start(args, format);
	retval = print(NULL, NULL, format, args);
	va_end(args);

	return retval;
}

void __noreturn panic(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	print(NULL, NULL, format, args);
	va_end(args);

	zstage_hang();
}

const struct zstage_console_device *zstage_console_get_device(void)
{
	return console_dev;
}

void zstage_console_set_device(const struct zstage_console_device *dev)
{
	if (!dev || console_dev)
		return;

	console_dev = dev;
}

int zstage_console_init(unsigned long boot_arg1)
{
	return platform_stdio_init(boot_arg1);
}
