/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <zstage/image_header.h>
#include <zstage/riscv_asm.h>
#include <zstage/riscv_encoding.h>

/*
 * If __start_reg <= __check_reg and __check_reg < __end_reg then
 *   jump to __pass
 */
.macro BRANGE __start_reg, __end_reg, __check_reg, __jump_lable
	blt	\__check_reg, \__start_reg, 999f
	bge	\__check_reg, \__end_reg, 999f
	j	\__jump_lable
999:
.endm

	.section .entry, "ax", %progbits
	.align 3
	.globl _start
_start:
	j	_start_real
_zstage_header_magic:
	.word		ZSTAGE_IMAGE_MAGIC
_zstage_header_version:
	.word		ZSTAGE_IMAGE_VERSION
_zstage_header_reserved1:
	.word		0
_zstage_header_image_run_addr:
	.dword		_zstage_start
_zstage_header_image_size:
	.dword		(_zstage_end - _zstage_start)
_zstage_header_image_spin_addr:
	.dword		_zstage_spin_addr
_zstage_header_image_nmivect_addr:
	.dword		_zstage_nmivect
_zstage_header_image_nmivect_size:
	.dword		(_zstage_nmivect_end - _zstage_nmivect)
#ifdef ZSTAGE_PLATDATA_SIZE
_zstage_header_image_platdata_addr:
	.dword		_zstage_platdata
_zstage_header_image_platdata_size:
	.dword		(_zstage_platdata_end - _zstage_platdata)
#else
_zstage_header_image_platdata_addr:
	.dword		0
_zstage_header_image_platdata_size:
	.dword		0
#endif
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
	.dword		0, 0, 0, 0, 0, 0, 0, 0
	.dword		0, 0, 0

	/* Secondary CPU spin address at ZSTAGE_TEXT_START + 4KB */
	.align 12
_zstage_spin_addr:
	RISCV_PTR	0

	/* NMI vector address at ZSTAGE_TEXT_START + (2 * 4KB) */
	.align 12
_zstage_nmivect:
	wfi
	j	_zstage_nmivect
	.align 12
_zstage_nmivect_end:

	/* Real start at ZSTAGE_TEXT_START + (3 * 4KB) */
	.align 12
_start_real:
	/* Disable and clear all interrupts */
	csrw	CSR_MIE, zero
	csrw	CSR_MIP, zero

	/* Setup exception vector */
	la	t0, _start_hang
	csrw	CSR_MTVEC, t0

	/* Platform specific HART initialization */
	call	_zstate_platform_hart_init

	/* Jump to relocation wait loop if we don't get relocation lottery */
	la	t0, _zstage_relocate_lottery
	li	t1, 1
	amoadd.w t0, t1, (t0)
	bnez	t0, _wait_relocate_copy_done

	/* Save load address */
	la	t0, _zstage_load_start
	la	t1, _start
	REG_S	t1, 0(t0)

	/* Relocate if load address != link address */
_relocate:
	la	t0, _zstage_header_image_run_addr
	REG_L	t0, 0(t0)
	la	t1, _zstage_header_image_size
	REG_L	t1, 0(t1)
	la	t2, _zstage_load_start
	REG_L	t2, 0(t2)
	add	t3, t2, t1
	add	t1, t1, t0
	beq	t0, t2, _relocate_done
	la	t4, _relocate_done
	sub	t4, t4, t2
	add	t4, t4, t0
	blt	t2, t0, _relocate_copy_to_upper
_relocate_copy_to_lower:
	ble	t1, t2, _relocate_copy_to_lower_loop
	la	t3, _zstage_relocate_lottery
	BRANGE	t2, t1, t3, _start_hang
	la	t3, _zstage_boot_status
	BRANGE	t2, t1, t3, _start_hang
	la	t3, _relocate
	la	t5, _relocate_done
	BRANGE	t2, t1, t3, _start_hang
	BRANGE	t2, t1, t5, _start_hang
	BRANGE  t3, t5, t2, _start_hang
_relocate_copy_to_lower_loop:
	REG_L	t3, 0(t2)
	REG_S	t3, 0(t0)
	add	t0, t0, __SIZEOF_POINTER__
	add	t2, t2, __SIZEOF_POINTER__
	blt	t0, t1, _relocate_copy_to_lower_loop
	jr	t4
_relocate_copy_to_upper:
	ble	t3, t0, _relocate_copy_to_upper_loop
	la	t2, _zstage_relocate_lottery
	BRANGE	t0, t3, t2, _start_hang
	la	t2, _zstage_boot_status
	BRANGE	t0, t3, t2, _start_hang
	la	t2, _relocate
	la	t5, _relocate_done
	BRANGE	t0, t3, t2, _start_hang
	BRANGE	t0, t3, t5, _start_hang
	BRANGE	t2, t5, t0, _start_hang
_relocate_copy_to_upper_loop:
	add	t3, t3, -__SIZEOF_POINTER__
	add	t1, t1, -__SIZEOF_POINTER__
	REG_L	t2, 0(t3)
	REG_S	t2, 0(t1)
	blt	t0, t1, _relocate_copy_to_upper_loop
	jr	t4
_wait_relocate_copy_done:
	la	t0, _start
	la	t1, _zstage_header_image_run_addr
	REG_L	t1, 0(t1)
	beq	t0, t1, _secondary_spin_loop
	la	t2, _zstage_boot_status
	la	t3, _secondary_spin_loop
	sub	t3, t3, t0
	add	t3, t3, t1
1:	/* Waiting for relocate copy done (_zstage_boot_status == 1) */
	li	t4, 1
	REG_L	t5, 0(t2)
	/* Reduce the bus traffic so that boot hart may proceed faster */
	nop
	nop
	nop
	bgt	t4, t5, 1b
	jr	t3
_relocate_done:

	/*
	 * Mark relocate copy done
	 * Use _zstage_boot_status copy relative to the load address
	 */
	la	t0, _zstage_boot_status
	la	t1, _zstage_header_image_run_addr
	REG_L	t1, 0(t1)
	la	t2, _zstage_load_start
	REG_L	t2, 0(t2)
	sub	t0, t0, t1
	add	t0, t0, t2
	li	t1, 1
	REG_S	t1, 0(t0)
	fence	rw, rw

	/* Save boot arg1 */
	la	t0, _zstage_boot_arg1
	REG_S	a1, (t0)

	/* Zero-out BSS section */
	la	t1, _bss_start
	la	t2, _bss_end
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

#ifdef ZSTAGE_NO_SPIN
	/* Make all secondary CPUs jump to next booting stage */
	la	t0, _zstage_spin_addr
	la	t1, _zstage_header_next_run_addr
	REG_L	t1, (t1)
	REG_S	t1, (t0)
	fence	rw, rw
#endif

	/* Jump to next booting stage */
	csrr	a0, CSR_MHARTID
	la	t0, _zstage_header_next_fdt_addr
	REG_L	a1, (t0)
	la	t0, _zstage_header_next_run_addr
	REG_L	t0, (t0)
	jalr	t0

	/* Wait for zStage spin address to become non-zero */
_secondary_spin_loop:
	la	t0, _zstage_spin_addr
_secondary_spin_loop_again:
	/* Reage zStage done flag */
	REG_L	t1, (t0)
	/* Few NOPs */
	add	zero, zero, zero
	add	zero, zero, zero
	add	zero, zero, zero
	add	zero, zero, zero
	/* Check zStage done flag */
	beq	t1, zero, _secondary_spin_loop_again
	/* Jump to next location */
	csrr	a0, CSR_MHARTID
	la	t0, _zstage_header_next_fdt_addr
	REG_L	a1, (t0)
	jalr	t1

	.section .entry, "ax", %progbits
	.align 3
	.globl _start_hang
_start_hang:
	wfi
	j	_start_hang

	.section .entry, "ax", %progbits
	.align 3
	.weak _zstate_platform_hart_init
_zstate_platform_hart_init:
	ret

	.data
	.align 3
_zstage_relocate_lottery:
	RISCV_PTR	0
_zstage_load_start:
	RISCV_PTR	0
_zstage_boot_arg1:
	RISCV_PTR	0
_zstage_boot_status:
	RISCV_PTR	0
_zstage_stack_top:
	RISCV_PTR	_stack_top

#ifdef ZSTAGE_PLATDATA_SIZE
	.bss
#ifdef ZSTAGE_PLATDATA_ALIGN
	.align ZSTAGE_PLATDATA_ALIGN
#else
	.align 12
#endif
_zstage_platdata:
	.rept		ZSTAGE_PLATDATA_SIZE
	.byte		0
	.endr
_zstage_platdata_end:
#endif

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
