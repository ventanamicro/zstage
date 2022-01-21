#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
# Copyright (c) 2022 Ventana Micro Systems Inc.
#

# Select Make Options:
MAKEFLAGS += -r --no-print-directory

# Use template platform if not specified
PLATFORM ?= generic

# Find out source, build, and install directories
src_dir=$(CURDIR)
ifdef O
 build_dir=$(shell $(READLINK) -f $(O))
else
 build_dir=$(CURDIR)/build
endif
ifeq ($(build_dir),$(CURDIR))
$(error Build directory is same as source directory.)
endif
install_root_dir_default=$(CURDIR)/install
ifdef I
 install_root_dir=$(shell $(READLINK) -f $(I))
else
 install_root_dir=$(install_root_dir_default)
endif
ifeq ($(install_root_dir),$(CURDIR))
$(error Install root directory is same as source directory.)
endif
ifeq ($(install_root_dir),$(build_dir))
$(error Install root directory is same as build directory.)
endif

# Check if verbosity is ON for build process
CMD_PREFIX_DEFAULT := @
ifeq ($(V), 1)
	CMD_PREFIX :=
else
	CMD_PREFIX := $(CMD_PREFIX_DEFAULT)
endif

# Setup path of directories
export platform_subdir=$(PLATFORM)
export platform_src_dir=$(src_dir)/platform/$(platform_subdir)
export platform_build_dir=$(build_dir)/platform/$(platform_subdir)
export include_dir=$(CURDIR)/include
export firmware_dir=$(CURDIR)/firmware

# Find library version
ZSTAGE_VERSION_MAJOR=`grep "define ZSTAGE_VERSION_MAJOR" $(include_dir)/zstage/version.h | sed 's/.*MAJOR.*\([0-9][0-9]*\)/\1/'`
ZSTAGE_VERSION_MINOR=`grep "define ZSTAGE_VERSION_MINOR" $(include_dir)/zstage/version.h | sed 's/.*MINOR.*\([0-9][0-9]*\)/\1/'`
ZSTAGE_VERSION_GIT=$(shell if [ -d $(src_dir)/.git ]; then git describe 2> /dev/null; fi)

# Setup compilation commands
ifdef CROSS_COMPILE
CC		=	$(CROSS_COMPILE)gcc
AR		=	$(CROSS_COMPILE)ar
LD		=	$(CROSS_COMPILE)ld
OBJCOPY		=	$(CROSS_COMPILE)objcopy
else
CC		?=	gcc
AR		?=	ar
LD		?=	ld
OBJCOPY		?=	objcopy
endif
CPP		=	$(CC) -E
AS		=	$(CC)
DTC		=	dtc

# Guess the compiler's XLEN
ZSTAGE_CC_XLEN := $(shell TMP=`$(CC) -dumpmachine | sed 's/riscv\([0-9][0-9]\).*/\1/'`; echo $${TMP})

# Setup platform XLEN
ifndef PLATFORM_RISCV_XLEN
  ifeq ($(ZSTAGE_CC_XLEN), 32)
    PLATFORM_RISCV_XLEN = 32
  else
    PLATFORM_RISCV_XLEN = 64
  endif
endif

# Build Info:
# ZSTAGE_BUILD_TIME_STAMP -- the compilation time stamp
# ZSTAGE_BUILD_COMPILER_VERSION -- the compiler version info
BUILD_INFO ?= 0
ifeq ($(BUILD_INFO),1)
ZSTAGE_BUILD_DATE_FMT = +%Y-%m-%d %H:%M:%S %z
ifdef SOURCE_DATE_EPOCH
	ZSTAGE_BUILD_TIME_STAMP ?= $(shell date -u -d "@$(SOURCE_DATE_EPOCH)" \
		"$(ZSTAGE_BUILD_DATE_FMT)" 2>/dev/null || \
		date -u -r "$(SOURCE_DATE_EPOCH)" \
		"$(ZSTAGE_BUILD_DATE_FMT)" 2>/dev/null || \
		date -u "$(ZSTAGE_BUILD_DATE_FMT)")
else
	ZSTAGE_BUILD_TIME_STAMP ?= $(shell date "$(ZSTAGE_BUILD_DATE_FMT)")
endif
ZSTAGE_BUILD_COMPILER_VERSION=$(shell $(CC) -v 2>&1 | grep ' version ' | \
	sed 's/[[:space:]]*$$//')
endif

# Setup list of objects.mk files
platform-object-mks=$(shell if [ -d $(platform_src_dir)/ ]; then find $(platform_src_dir) -iname "objects.mk" | sort -r; fi)
firmware-object-mks=$(shell if [ -d $(firmware_dir) ]; then find $(firmware_dir) -iname "objects.mk" | sort -r; fi)

# Include platform config.mk
include $(platform_src_dir)/config.mk

# Include all object.mk files
include $(platform-object-mks)
include $(firmware-object-mks)

# Setup list of objects
platform-objs-path-y=$(foreach obj,$(platform-objs-y),$(platform_build_dir)/$(obj))
platform-dtbs-path-y=$(foreach dtb,$(platform-dtbs-y),$(platform_build_dir)/$(dtb))
firmware-objs-path-y=$(foreach obj,$(firmware-objs-y),$(platform_build_dir)/firmware/$(obj))
all-objs-path-y=$(platform-objs-path-y) $(firmware-objs-path-y)

# Setup list of deps files for objects
deps-y=$(platform-objs-path-y:.o=.dep)
deps-y+=$(firmware-objs-path-y:.o=.dep)

# Setup platform ABI, ISA and Code Model
ifndef PLATFORM_RISCV_ABI
  ifneq ($(PLATFORM_RISCV_TOOLCHAIN_DEFAULT), 1)
    ifeq ($(PLATFORM_RISCV_XLEN), 32)
      PLATFORM_RISCV_ABI = ilp$(PLATFORM_RISCV_XLEN)
    else
      PLATFORM_RISCV_ABI = lp$(PLATFORM_RISCV_XLEN)
    endif
  else
    PLATFORM_RISCV_ABI = $(ZSTAGE_CC_ABI)
  endif
endif
ifndef PLATFORM_RISCV_ISA
  ifneq ($(PLATFORM_RISCV_TOOLCHAIN_DEFAULT), 1)
    PLATFORM_RISCV_ISA = rv$(PLATFORM_RISCV_XLEN)imafdc
  else
    PLATFORM_RISCV_ISA = $(ZSTAGE_CC_ISA)
  endif
endif
ifndef PLATFORM_RISCV_CODE_MODEL
  PLATFORM_RISCV_CODE_MODEL = medany
endif

# Setup compilation commands flags
GENFLAGS	+=	-I$(platform_src_dir)/include
GENFLAGS	+=	-I$(include_dir)
ifneq ($(ZSTAGE_VERSION_GIT),)
GENFLAGS	+=	-DZSTAGE_VERSION_GIT="\"$(ZSTAGE_VERSION_GIT)\""
endif
ifeq ($(BUILD_INFO),1)
GENFLAGS	+=	-DZSTAGE_BUILD_TIME_STAMP="\"$(ZSTAGE_BUILD_TIME_STAMP)\""
GENFLAGS	+=	-DZSTAGE_BUILD_COMPILER_VERSION="\"$(ZSTAGE_BUILD_COMPILER_VERSION)\""
endif
GENFLAGS	+=	$(platform-genflags-y)
GENFLAGS	+=	$(firmware-genflags-y)

CFLAGS		=	-g -Wall -Werror -ffreestanding -nostdlib -fno-stack-protector -fno-strict-aliasing -O2
CFLAGS		+=	-fno-omit-frame-pointer -fno-optimize-sibling-calls -mstrict-align
CFLAGS		+=	-mno-save-restore
CFLAGS		+=	-mabi=$(PLATFORM_RISCV_ABI) -march=$(PLATFORM_RISCV_ISA)
CFLAGS		+=	-mcmodel=$(PLATFORM_RISCV_CODE_MODEL)
CFLAGS		+=	$(GENFLAGS)
CFLAGS		+=	$(platform-cflags-y)
CFLAGS		+=	-fno-pie -no-pie
CFLAGS		+=	$(firmware-cflags-y)

CPPFLAGS	+=	$(GENFLAGS)
CPPFLAGS	+=	$(platform-cppflags-y)
CPPFLAGS	+=	$(firmware-cppflags-y)

ASFLAGS		=	-g -Wall -nostdlib
ASFLAGS		+=	-fno-omit-frame-pointer -fno-optimize-sibling-calls -mstrict-align
ASFLAGS		+=	-mno-save-restore
ASFLAGS		+=	-mabi=$(PLATFORM_RISCV_ABI) -march=$(PLATFORM_RISCV_ISA)
ASFLAGS		+=	-mcmodel=$(PLATFORM_RISCV_CODE_MODEL)
ASFLAGS		+=	$(GENFLAGS)
ASFLAGS		+=	$(platform-asflags-y)
ASFLAGS		+=	$(firmware-asflags-y)

ARFLAGS		=	rcs

ELFFLAGS	+=	-Wl,--build-id=none -Wl,-N
ELFFLAGS	+=	$(platform-ldflags-y)
ELFFLAGS	+=	$(firmware-ldflags-y)

MERGEFLAGS	+=	-r
MERGEFLAGS	+=	-b elf$(PLATFORM_RISCV_XLEN)-littleriscv
MERGEFLAGS	+=	-m elf$(PLATFORM_RISCV_XLEN)lriscv

DTSCPPFLAGS	=	$(CPPFLAGS) -nostdinc -nostdlib -fno-builtin -D__DTS__ -x assembler-with-cpp

# Setup functions for compilation
define dynamic_flags
-I$(shell dirname $(2)) -D__OBJNAME__=$(subst -,_,$(shell basename $(1) .o))
endef
merge_objs = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " MERGE     $(subst $(build_dir)/,,$(1))"; \
	     $(LD) $(MERGEFLAGS) $(2) -o $(1)
merge_deps = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " MERGE-DEP $(subst $(build_dir)/,,$(1))"; \
	     cat $(2) > $(1)
copy_file =  $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " COPY      $(subst $(build_dir)/,,$(1))"; \
	     cp -f $(2) $(1)
inst_file =  $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " INSTALL   $(subst $(install_root_dir)/,,$(1))"; \
	     cp -f $(2) $(1)
inst_file_list = $(CMD_PREFIX)if [ ! -z "$(4)" ]; then \
	     mkdir -p $(1)/$(3); \
	     for file in $(4) ; do \
	     rel_file=`echo $$file | sed -e 's@$(2)/@@'`; \
	     dest_file=$(1)"/"$(3)"/"`echo $$rel_file`; \
	     dest_dir=`dirname $$dest_file`; \
	     echo " INSTALL   "$(3)"/"`echo $$rel_file`; \
	     mkdir -p $$dest_dir; \
	     cp -f $$file $$dest_file; \
	     done \
	     fi
compile_cpp = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " CPP       $(subst $(build_dir)/,,$(1))"; \
	     $(CPP) $(CPPFLAGS) -x c $(2) | grep -v "\#" > $(1)
compile_cc_dep = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " CC-DEP    $(subst $(build_dir)/,,$(1))"; \
	     printf %s `dirname $(1)`/  > $(1) && \
	     $(CC) $(CFLAGS) $(call dynamic_flags,$(1),$(2))   \
	       -MM $(2) >> $(1) || rm -f $(1)
compile_cc = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " CC        $(subst $(build_dir)/,,$(1))"; \
	     $(CC) $(CFLAGS) $(call dynamic_flags,$(1),$(2)) -c $(2) -o $(1)
compile_as_dep = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " AS-DEP    $(subst $(build_dir)/,,$(1))"; \
	     printf %s `dirname $(1)`/ > $(1) && \
	     $(AS) $(ASFLAGS) $(call dynamic_flags,$(1),$(2)) \
	       -MM $(2) >> $(1) || rm -f $(1)
compile_as = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " AS        $(subst $(build_dir)/,,$(1))"; \
	     $(AS) $(ASFLAGS) $(call dynamic_flags,$(1),$(2)) -c $(2) -o $(1)
compile_elf = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " ELF       $(subst $(build_dir)/,,$(1))"; \
	     $(CC) $(CFLAGS) $(3) $(ELFFLAGS) -Wl,-T$(2) -o $(1)
compile_ar = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " AR        $(subst $(build_dir)/,,$(1))"; \
	     $(AR) $(ARFLAGS) $(1) $(2)
compile_objcopy = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " OBJCOPY   $(subst $(build_dir)/,,$(1))"; \
	     $(OBJCOPY) -S -O binary $(2) $(1)
compile_dts = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " DTC       $(subst $(build_dir)/,,$(1))"; \
	     $(CPP) $(DTSCPPFLAGS) $(2) | $(DTC) -O dtb -i `dirname $(2)` -o $(1)
compile_d2c = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " D2C       $(subst $(build_dir)/,,$(1))"; \
	     $(if $($(2)-varalign-$(3)),$(eval D2C_ALIGN_BYTES := $($(2)-varalign-$(3))),$(eval D2C_ALIGN_BYTES := $(4))) \
	     $(if $($(2)-varprefix-$(3)),$(eval D2C_NAME_PREFIX := $($(2)-varprefix-$(3))),$(eval D2C_NAME_PREFIX := $(5))) \
	     $(if $($(2)-padding-$(3)),$(eval D2C_PADDING_BYTES := $($(2)-padding-$(3))),$(eval D2C_PADDING_BYTES := 0)) \
	     $(src_dir)/scripts/d2c.sh -i $(6) -a $(D2C_ALIGN_BYTES) -p $(D2C_NAME_PREFIX) -t $(D2C_PADDING_BYTES) > $(1)
compile_gen_dep = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " GEN-DEP   $(subst $(build_dir)/,,$(1))"; \
	     echo "$(1:.dep=$(2)): $(3)" >> $(1)

targets-y = $(platform-dtbs-path-y)
targets-y += $(platform_build_dir)/firmware/zstage.elf
targets-y += $(platform_build_dir)/firmware/zstage.bin

# Default rule "make" should always be first rule
.PHONY: all
all: $(targets-y)

# Preserve all intermediate files
.SECONDARY:

ifeq ($(BUILD_INFO),1)
$(platform_build_dir)/firmware/main.o: $(firmware_dir)/main.c FORCE
	$(call compile_cc,$@,$<)
endif

$(platform_build_dir)/%.bin: $(platform_build_dir)/%.elf
	$(call compile_objcopy,$@,$<)

$(platform_build_dir)/%.elf: $(platform_build_dir)/%.elf.ld $(all-objs-path-y)
	$(call compile_elf,$@,$<,$(all-objs-path-y))

$(platform_build_dir)/%.ld: $(src_dir)/%.ldS
	$(call compile_cpp,$@,$<)

$(platform_build_dir)/%.dep: $(platform_src_dir)/%.c
	$(call compile_cc_dep,$@,$<)

$(platform_build_dir)/%.o: $(platform_src_dir)/%.c
	$(call compile_cc,$@,$<)

$(platform_build_dir)/%.o: $(platform_build_dir)/%.c
	$(call compile_cc,$@,$<)

$(platform_build_dir)/%.dep: $(platform_src_dir)/%.S
	$(call compile_as_dep,$@,$<)

$(platform_build_dir)/%.o: $(platform_src_dir)/%.S
	$(call compile_as,$@,$<)

$(platform_build_dir)/%.dep: $(platform_src_dir)/%.dts
	$(call compile_gen_dep,$@,.dtb,$<)
	$(call compile_gen_dep,$@,.c,$(@:.dep=.dtb))
	$(call compile_gen_dep,$@,.o,$(@:.dep=.c))

$(platform_build_dir)/%.c: $(platform_build_dir)/%.dtb
	$(call compile_d2c,$@,platform,$(subst .dtb,.o,$(subst /,-,$(subst $(platform_build_dir)/,,$<))),16,dt,$<)

$(platform_build_dir)/%.dtb: $(platform_src_dir)/%.dts
	$(call compile_dts,$@,$<)

$(platform_build_dir)/%.dep: $(src_dir)/%.c
	$(call compile_cc_dep,$@,$<)

$(platform_build_dir)/%.o: $(src_dir)/%.c
	$(call compile_cc,$@,$<)

$(platform_build_dir)/%.dep: $(src_dir)/%.S
	$(call compile_as_dep,$@,$<)

$(platform_build_dir)/%.o: $(src_dir)/%.S
	$(call compile_as,$@,$<)

# Dependency files should only be included after default Makefile rules
# They should not be included for any "xxxconfig" or "xxxclean" rule
all-deps-1 = $(if $(findstring config,$(MAKECMDGOALS)),,$(deps-y))
all-deps-2 = $(if $(findstring clean,$(MAKECMDGOALS)),,$(all-deps-1))
-include $(all-deps-2)

# Include external dependency of firmwares after default Makefile rules
include $(src_dir)/firmware/external_deps.mk

# Rule for "make install"
.PHONY: install
install: $(targets-y)
	$(call inst_file_list,$(install_root_dir),$(platform_build_dir),$(platform_subdir),$(platform-dtbs-path-y))
	$(call inst_file_list,$(install_root_dir),$(platform_build_dir),$(platform_subdir),$(platform_build_dir)/firmware/zstage.elf)
	$(call inst_file_list,$(install_root_dir),$(platform_build_dir),$(platform_subdir),$(platform_build_dir)/firmware/zstage.bin)

# Rule for "make clean"
.PHONY: clean
clean:
	$(CMD_PREFIX)mkdir -p $(build_dir)
	$(if $(V), @echo " RM        $(build_dir)/*.o")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.o" -exec rm -rf {} +
	$(if $(V), @echo " RM        $(build_dir)/*.a")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.a" -exec rm -rf {} +
	$(if $(V), @echo " RM        $(build_dir)/*.elf")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.elf" -exec rm -rf {} +
	$(if $(V), @echo " RM        $(build_dir)/*.bin")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.bin" -exec rm -rf {} +
	$(if $(V), @echo " RM        $(build_dir)/*.dtb")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.dtb" -exec rm -rf {} +

# Rule for "make distclean"
.PHONY: distclean
distclean: clean
	$(CMD_PREFIX)mkdir -p $(build_dir)
	$(if $(V), @echo " RM        $(build_dir)/*.dep")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.dep" -exec rm -rf {} +
ifeq ($(build_dir),$(CURDIR)/build)
	$(if $(V), @echo " RM        $(build_dir)")
	$(CMD_PREFIX)rm -rf $(build_dir)
endif
ifeq ($(install_root_dir),$(install_root_dir_default))
	$(if $(V), @echo " RM        $(install_root_dir_default)")
	$(CMD_PREFIX)rm -rf $(install_root_dir_default)
endif

.PHONY: FORCE
FORCE:
