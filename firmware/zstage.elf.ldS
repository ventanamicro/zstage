/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
	. = ZSTAGE_TEXT_START;
	PROVIDE(_zstage_start = .);

	. = ALIGN(0x1000); /* Need this to create proper sections */

	/* Beginning of the code section */

	.text :
 	{
		PROVIDE(_text_start = .);
		*(.entry)
		*(.text)
		. = ALIGN(8);
		PROVIDE(_text_end = .);
	}

	. = ALIGN(0x1000); /* Ensure next section is page aligned */

	/* End of the code sections */

	/* Beginning of the read-only data sections */

	.rodata :
	{
		PROVIDE(_rodata_start = .);
		*(.rodata .rodata.*)
		. = ALIGN(8);
		PROVIDE(_rodata_end = .);
	}

	. = ALIGN(0x1000); /* Ensure next section is page aligned */

	/* End of the read-only data sections */

	/* Beginning of the read-write data sections */

	.data :
	{
		PROVIDE(_data_start = .);

		*(.sdata)
		*(.sdata.*)
		*(.data)
		*(.data.*)
		*(.readmostly.data)
		*(*.data)
		. = ALIGN(8);

		PROVIDE(_data_end = .);
	}

	. = ALIGN(0x1000); /* Ensure next section is page aligned */

	.bss :
	{
		PROVIDE(_bss_start = .);
		*(.sbss)
		*(.sbss.*)
		*(.bss)
		*(.bss.*)
		. = ALIGN(8);
		. = . + ZSTAGE_STACK_SIZE;
		PROVIDE(_stack_top = .);
		PROVIDE(_bss_end = .);
	}

	. = ALIGN(0x1000); /* Ensure next section is page aligned */

	.payload :
	{
		PROVIDE(_payload_start = .);
		*(.payload)
		. = ALIGN(8);
		PROVIDE(_payload_end = .);
	}

	. = ALIGN(0x1000); /* Ensure next section is page aligned */

	PROVIDE(_zstage_end = .);
}
