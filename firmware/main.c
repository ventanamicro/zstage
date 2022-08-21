/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <zstage/console.h>
#include <zstage/image_header.h>
#include <zstage/main.h>
#include <zstage/payload_blob.h>
#include <zstage/platform.h>
#include <zstage/riscv_asm.h>
#include <zstage/string.h>
#include <zstage/types.h>
#include <zstage/version.h>

void __noreturn zstage_hang(void)
{
	while (1)
		wfi();
	__builtin_unreachable();
}

void zstage_main(unsigned long boot_arg1,
		 const struct zstage_image_header *hdr,
		 const struct zstage_payload_blob *blobs)
{
	int rc;
	const struct zstage_console_device *cdev;

	/* Check header magic and version */
	if (hdr->magic != ZSTAGE_IMAGE_MAGIC ||
	    hdr->version != ZSTAGE_IMAGE_VERSION)
		zstage_hang();

	/* Do console init */
	rc = zstage_console_init(boot_arg1);
	if (rc)
		zstage_hang();

	/* Print banner */
#ifdef ZSTAGE_VERSION_GIT
	printf("\nVentana zStage %s\n", ZSTAGE_VERSION_GIT);
#else
	printf("\nVentana zStage v%d.%d\n", ZSTAGE_VERSION_MAJOR,
	       ZSTAGE_VERSION_MINOR);
#endif

#ifdef ZSTAGE_BUILD_TIME_STAMP
	printf("zstage: build time: %s\n", ZSTAGE_BUILD_TIME_STAMP);
#endif

#ifdef ZSTAGE_BUILD_COMPILER_VERSION
	printf("zstage: compiler: %s\n", ZSTAGE_BUILD_COMPILER_VERSION);
#endif

	/* Do platform early init */
	rc = platform_early_init(boot_arg1);
	if (rc) {
		printf("zstage: platform early init failed (error %d)\n", rc);
		zstage_hang();
	}

	/* Print console device name */
	cdev = zstage_console_get_device();
	printf("zstage: console device %s\n", (cdev) ? cdev->name : "---");

	/* Copy blobs to destination */
	while (blobs->size) {
		printf("zstage: copying %s (%lu bytes) to 0x%lx from 0x%lx\n",
		       "blob", blobs->size, blobs->dst_addr, blobs->src_addr);
		memcpy((void *)blobs->dst_addr,
		       (void *)blobs->src_addr, blobs->size);
		blobs++;
	}

	/* Prepare platform FDT */
	printf("zstage: setup platform FDT at 0x%lx\n", hdr->next_fdt_addr);
	rc = platform_prepare_fdt(boot_arg1, hdr);
	if (rc) {
		printf("zstage: platform FDT prepare failed (error %d)\n", rc);
		zstage_hang();
	}

	/* Copy next booting stage to run location */
	if (hdr->next_load_addr != hdr->next_run_addr) {
		printf("zstage: copying %s (%lu bytes) to 0x%lx from 0x%lx\n",
		       "next stage", hdr->next_size, hdr->next_run_addr,
		       hdr->next_load_addr);
		memcpy((void *)hdr->next_run_addr,
		       (void *)hdr->next_load_addr, hdr->next_size);
	}

	/* Print next booting stage location */
	printf("zstage: next booting stage at 0x%lx (%lu bytes)\n",
	       hdr->next_run_addr, hdr->next_size);

	/* Do platform final init */
	rc = platform_final_init(boot_arg1);
	if (rc) {
		printf("zstage: platform final init failed (error %d)\n", rc);
		zstage_hang();
	}

	/* Last print */
	printf("zstage: jumping to next booting stage\n");
}
