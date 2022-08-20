#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2022 Ventana Micro Systems Inc.
#

ifdef ZSTAGE_PAYLOAD_PATH
$(platform_build_dir)/firmware/head.o: $(ZSTAGE_PAYLOAD_PATH)
endif

ifdef ZSTAGE_PAYLOAD_BLOB1_PATH
$(platform_build_dir)/firmware/head.o: $(ZSTAGE_PAYLOAD_BLOB1_PATH)
endif

ifdef ZSTAGE_PAYLOAD_BLOB2_PATH
$(platform_build_dir)/firmware/head.o: $(ZSTAGE_PAYLOAD_BLOB2_PATH)
endif

ifdef ZSTAGE_PAYLOAD_BLOB3_PATH
$(platform_build_dir)/firmware/head.o: $(ZSTAGE_PAYLOAD_BLOB3_PATH)
endif
