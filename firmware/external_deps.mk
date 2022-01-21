#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2022 Ventana Micro Systems Inc.
#

ifdef ZSTAGE_PAYLOAD_PATH
$(platform_build_dir)/firmware/main.o: $(ZSTAGE_PAYLOAD_PATH)
endif

