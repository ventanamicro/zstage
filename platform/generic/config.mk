#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2022 Ventana Micro Systems Inc.
#

# Compiler flags
platform-cppflags-y =
platform-cflags-y =
platform-asflags-y =
platform-ldflags-y =

# Config options
ZSTAGE_TEXT_START=0x84000000
ZSTAGE_STACK_SIZE=0x4000
ZSTAGE_PAYLOAD_FDT_ADDR=0x83F00000
ZSTAGE_PAYLOAD_NEXT_ADDR=0x80000000
ZSTAGE_NO_SPIN=y
