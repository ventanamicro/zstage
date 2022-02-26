#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2022 Ventana Micro Systems Inc.
#

firmware-genflags-y =
firmware-cppflags-y +=
firmware-cflags-y +=
firmware-asflags-y +=
firmware-ldflags-y +=

ifdef ZSTAGE_TEXT_START
firmware-genflags-y += -DZSTAGE_TEXT_START=$(ZSTAGE_TEXT_START)
else
$(error Must define ZSTAGE_TEXT_START in platform config.mk.)
endif

ifdef ZSTAGE_STACK_SIZE
firmware-genflags-y += -DZSTAGE_STACK_SIZE=$(ZSTAGE_STACK_SIZE)
else
$(error Must define ZSTAGE_STACK_SIZE in platform config.mk.)
endif

ifdef ZSTAGE_PAYLOAD_FDT_ADDR
firmware-genflags-y += -DZSTAGE_PAYLOAD_FDT_ADDR=$(ZSTAGE_PAYLOAD_FDT_ADDR)
else
$(error Must define ZSTAGE_PAYLOAD_FDT_ADDR in platform config.mk.)
endif

ifdef ZSTAGE_PAYLOAD_NEXT_ADDR
firmware-genflags-y += -DZSTAGE_PAYLOAD_NEXT_ADDR=$(ZSTAGE_PAYLOAD_NEXT_ADDR)
else
$(error Must define ZSTAGE_PAYLOAD_NEXT_ADDR in platform config.mk.)
endif

ifdef ZSTAGE_PAYLOAD_PATH
firmware-genflags-y += -DZSTAGE_PAYLOAD_PATH=\"$(ZSTAGE_PAYLOAD_PATH)\"
endif

ifdef ZSTAGE_NO_SPIN
firmware-genflags-y += -DZSTAGE_NO_SPIN=1
endif

firmware-objs-y += head.o
firmware-objs-y += main.o
firmware-objs-y += string.o
firmware-objs-y += console.o
