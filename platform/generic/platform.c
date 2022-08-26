/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <libfdt.h>
#include <zstage/error.h>
#include <zstage/image_header.h>
#include <zstage/serial/fdt_serial.h>
#include <zstage/serial/semihosting.h>
#include <zstage/platform.h>
#include <zstage/string.h>

int platform_early_init(unsigned long boot_arg1)
{
	/* Nothing to do here. */
	return 0;
}

int platform_stdio_init(unsigned long boot_arg1)
{
	/*
	 * First try semihosting. If it works then
	 * don't try FDT based serial drivers
	 */
	if (!semihosting_serial_init())
		return 0;

	/* Probe FDT based serial drivers. */
	return fdt_serial_init((void *)boot_arg1);
}

int platform_prepare_fdt(unsigned long boot_arg1,
			 const struct zstage_image_header *hdr)
{
	void *fdt = (void *)boot_arg1;

	if (fdt_magic(fdt) != FDT_MAGIC)
		return -ENOSYS;

	/* Copy FDT to target address */
	memcpy((void *)hdr->next_fdt_addr, fdt, fdt_totalsize(fdt));
	return 0;
}

int platform_final_init(unsigned long boot_arg1)
{
	/* Nothing to do here. */
	return 0;
}
