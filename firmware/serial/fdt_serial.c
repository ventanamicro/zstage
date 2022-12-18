/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 */

#include <libfdt.h>
#include <zstage/error.h>
#include <zstage/fdt/fdt_helper.h>
#include <zstage/serial/fdt_serial.h>

extern struct fdt_serial fdt_serial_cadence;
extern struct fdt_serial fdt_serial_uart8250;
extern struct fdt_serial fdt_serial_htif;

static struct fdt_serial *serial_drivers[] = {
	&fdt_serial_cadence,
	&fdt_serial_uart8250,
	&fdt_serial_htif,
};

static struct fdt_serial dummy = {
	.match_table = NULL,
	.init = NULL,
};

static struct fdt_serial *current_driver = &dummy;

int fdt_serial_init(void *fdt)
{
	const void *prop;
	struct fdt_serial *drv;
	const struct fdt_match *match;
	int pos, noff = -1, len, coff, rc;

	/* Find offset of node pointed to by stdout-path */
	coff = fdt_path_offset(fdt, "/chosen");
	if (-1 < coff) {
		prop = fdt_getprop(fdt, coff, "stdout-path", &len);
		if (prop && len) {
			const char *sep, *start = prop;

			/* The device path may be followed by ':' */
			sep = strchr(start, ':');
			if (sep)
				noff = fdt_path_offset_namelen(fdt, prop,
							       sep - start);
			else
				noff = fdt_path_offset(fdt, prop);
		}
	}

	/* First check DT node pointed by stdout-path */
	for (pos = 0; pos < array_size(serial_drivers) && -1 < noff; pos++) {
		drv = serial_drivers[pos];

		match = fdt_match_node(fdt, noff, drv->match_table);
		if (!match)
			continue;

		if (drv->init) {
			rc = drv->init(fdt, noff, match);
			if (rc == -ENODEV)
				continue;
			if (rc)
				return rc;
		}
		current_driver = drv;
		break;
	}

	/* Check if we found desired driver */
	if (current_driver != &dummy)
		goto done;

	/* Lastly check all DT nodes */
	for (pos = 0; pos < array_size(serial_drivers); pos++) {
		drv = serial_drivers[pos];

		noff = fdt_find_match(fdt, -1, drv->match_table, &match);
		if (noff < 0)
			continue;

		if (drv->init) {
			rc = drv->init(fdt, noff, match);
			if (rc == -ENODEV)
				continue;
			if (rc)
				return rc;
		}
		current_driver = drv;
		break;
	}

done:
	return 0;
}
