/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#ifndef __ZSTAGE_RISCV_ASM_H__
#define __ZSTAGE_RISCV_ASM_H__

#include <zstage/riscv_encoding.h>

/* clang-format off */

#ifdef __ASSEMBLER__
#define __ASM_STR(x)	x
#else
#define __ASM_STR(x)	#x
#endif

#if __riscv_xlen == 64
#define __REG_SEL(a, b)	__ASM_STR(a)
#elif __riscv_xlen == 32
#define __REG_SEL(a, b)	__ASM_STR(b)
#else
#error "Unexpected __riscv_xlen"
#endif

#define PAGE_SHIFT	(12)
#define PAGE_SIZE	(_AC(1, UL) << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE - 1))

#define REG_L		__REG_SEL(ld, lw)
#define REG_S		__REG_SEL(sd, sw)
#define SZREG		__REG_SEL(8, 4)
#define LGREG		__REG_SEL(3, 2)

#if __SIZEOF_POINTER__ == 8
#ifdef __ASSEMBLER__
#define RISCV_PTR		.dword
#define RISCV_SZPTR		8
#define RISCV_LGPTR		3
#else
#define RISCV_PTR		".dword"
#define RISCV_SZPTR		"8"
#define RISCV_LGPTR		"3"
#endif
#elif __SIZEOF_POINTER__ == 4
#ifdef __ASSEMBLER__
#define RISCV_PTR		.word
#define RISCV_SZPTR		4
#define RISCV_LGPTR		2
#else
#define RISCV_PTR		".word"
#define RISCV_SZPTR		"4"
#define RISCV_LGPTR		"2"
#endif
#else
#error "Unexpected __SIZEOF_POINTER__"
#endif

#if (__SIZEOF_INT__ == 4)
#define RISCV_INT		__ASM_STR(.word)
#define RISCV_SZINT		__ASM_STR(4)
#define RISCV_LGINT		__ASM_STR(2)
#else
#error "Unexpected __SIZEOF_INT__"
#endif

#if (__SIZEOF_SHORT__ == 2)
#define RISCV_SHORT		__ASM_STR(.half)
#define RISCV_SZSHORT		__ASM_STR(2)
#define RISCV_LGSHORT		__ASM_STR(1)
#else
#error "Unexpected __SIZEOF_SHORT__"
#endif

/* clang-format on */

#ifndef __ASSEMBLER__

#define csr_swap(csr, val)                                              \
	({                                                              \
		unsigned long __v = (unsigned long)(val);               \
		__asm__ __volatile__("csrrw %0, " __ASM_STR(csr) ", %1" \
				     : "=r"(__v)                        \
				     : "rK"(__v)                        \
				     : "memory");                       \
		__v;                                                    \
	})

#define csr_read(csr)                                           \
	({                                                      \
		register unsigned long __v;                     \
		__asm__ __volatile__("csrr %0, " __ASM_STR(csr) \
				     : "=r"(__v)                \
				     :                          \
				     : "memory");               \
		__v;                                            \
	})

#define csr_write(csr, val)                                        \
	({                                                         \
		unsigned long __v = (unsigned long)(val);          \
		__asm__ __volatile__("csrw " __ASM_STR(csr) ", %0" \
				     :                             \
				     : "rK"(__v)                   \
				     : "memory");                  \
	})

#define csr_read_set(csr, val)                                          \
	({                                                              \
		unsigned long __v = (unsigned long)(val);               \
		__asm__ __volatile__("csrrs %0, " __ASM_STR(csr) ", %1" \
				     : "=r"(__v)                        \
				     : "rK"(__v)                        \
				     : "memory");                       \
		__v;                                                    \
	})

#define csr_set(csr, val)                                          \
	({                                                         \
		unsigned long __v = (unsigned long)(val);          \
		__asm__ __volatile__("csrs " __ASM_STR(csr) ", %0" \
				     :                             \
				     : "rK"(__v)                   \
				     : "memory");                  \
	})

#define csr_read_clear(csr, val)                                        \
	({                                                              \
		unsigned long __v = (unsigned long)(val);               \
		__asm__ __volatile__("csrrc %0, " __ASM_STR(csr) ", %1" \
				     : "=r"(__v)                        \
				     : "rK"(__v)                        \
				     : "memory");                       \
		__v;                                                    \
	})

#define csr_clear(csr, val)                                        \
	({                                                         \
		unsigned long __v = (unsigned long)(val);          \
		__asm__ __volatile__("csrc " __ASM_STR(csr) ", %0" \
				     :                             \
				     : "rK"(__v)                   \
				     : "memory");                  \
	})

#define wfi()                                             \
	do {                                              \
		__asm__ __volatile__("wfi" ::: "memory"); \
	} while (0)

#define ebreak()                                             \
	do {                                              \
		__asm__ __volatile__("ebreak" ::: "memory"); \
	} while (0)

/* Get current HART id */
#define current_hartid()	((unsigned int)csr_read(CSR_MHARTID))

#endif /* !__ASSEMBLER__ */

#endif
