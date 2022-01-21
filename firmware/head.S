/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <zstage/image_header.h>
#include <zstage/riscv_asm.h>
#include <zstage/riscv_encoding.h>

	.section .entry, "ax", %progbits
	.align 3
	.globl _start
_start:
#ifdef __riscv_c
	j	_start_real
	nop
#else
	j	_start_real
#endif
_zstage_header_magic:
	.word		ZSTAGE_IMAGE_MAGIC
_zstage_header_version:
	.word		ZSTAGE_IMAGE_VERSION
_zstage_header_boot_device:
	.word		ZSTAGE_IMAGE_BOOTDEV_RAM
_zstage_header_image_run_addr:
	.dword		_zstage_start
_zstage_header_image_size:
	.dword		(_zstage_end - _zstage_start)
_zstage_header_next_load_addr:
	.dword		_zstage_payload_bin
_zstage_header_next_run_addr:
	.dword		ZSTAGE_PAYLOAD_NEXT_ADDR
_zstage_header_next_fdt_addr:
	.dword		ZSTAGE_PAYLOAD_FDT_ADDR
_zstage_header_next_size:
	.dword		(_zstage_payload_bin_end - _zstage_payload_bin)
_zstage_header_reserved2:
	.dword		0, 0, 0, 0, 0, 0, 0, 0

_start_real:
	/* Disable and clear all interrupts */
	csrw	CSR_MIE, zero
	csrw	CSR_MIP, zero

	/* Setup exception vector */
	la	t0, _start_hang
	csrw	CSR_MTVEC, t0

	/* Ensure that we are booted from right address */
	la	t0, _start
	la	t1, _zstage_header_image_run_addr
	REG_L	t1, (t1)
	bne	t0, t1, _start_hang

	/* Jump to relocation wait loop if we don't get relocation lottery */
	la	t0, _zstage_relocate_lottery
	li	t1, 1
	amoadd.w t0, t1, (t0)
	bnez	t0, _wait_for_zstage_done

	/* Save boot arg1 */
	la	t0, _zstage_boot_arg1
	REG_S	a1, (t0)

	/* Zero-out BSS section */
	lla	t1, _bss_start
	lla	t2, _bss_end
_bss_zero:
	REG_S	zero, (t1)
	add	t1, t1, __SIZEOF_POINTER__
	blt	t1, t2, _bss_zero

	/* Call the main function. */
	la	t0, _zstage_stack_top
	REG_L	sp, (t0)
	la	a0, _zstage_boot_arg1
	REG_L	a0, (a0)
	la	a1, _start
	call	zstage_main

	/* set zStage done flag */
	li	t1, 1
	la	t0, _zstage_done
	REG_S	t1, (t0)

	/* Continue with next booting stage */
	j	_next_booting_stage

	/* Wait for zStage done loop */
_wait_for_zstage_done:
	la	t0, _zstage_done
_wait_for_zstage_again:
	/* Reage zStage done flag */
	REG_L	t1, (t0)
	/* Few NOPs */
	add	zero, zero, zero
	add	zero, zero, zero
	add	zero, zero, zero
	add	zero, zero, zero
	/* Check zStage done flag */
	beq	t1, zero, _wait_for_zstage_again

	/* Jump to next booting stage */
_next_booting_stage:
	csrr	a0, CSR_MHARTID
	la	t0, _zstage_header_next_fdt_addr
	REG_L	a1, (t0)
	la	t0, _zstage_header_next_run_addr
	REG_L	t0, (t0)
	jalr	t0

	.section .entry, "ax", %progbits
	.align 3
	.globl _start_hang
_start_hang:
	wfi
	j	_start_hang

	.data
	.align 3
_zstage_relocate_lottery:
	RISCV_PTR	0
_zstage_boot_arg1:
	RISCV_PTR	0
_zstage_done:
	RISCV_PTR	0
_zstage_stack_top:
	RISCV_PTR	_stack_top

	.section .payload, "ax", %progbits
	.align 4
_zstage_payload_bin:
#ifndef ZSTAGE_PAYLOAD_PATH
	wfi
	nop
	j	_zstage_payload_bin
#else
	.incbin	ZSTAGE_PAYLOAD_PATH
#endif
_zstage_payload_bin_end:
