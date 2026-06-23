# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023 Intel Corporation.

# If called directly from the command line, invoke the kernel build system.
ifeq ($(KERNELRELEASE),)
PWD = $(shell pwd)
RPM_PKG := $(shell rpm -qa)
KERNEL_SOURCE = /lib/modules/$(shell uname -r)/build

BIN := cet_ioctl

all: $(BIN)

cet_ioctl:
	$(MAKE) -C $(KERNEL_SOURCE) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SOURCE) M=$(PWD) clean
	${RM} -rf $(BIN) fuzz_cet_app

# Fuzz targets (user-mode, no kernel headers needed)
CLANG ?= clang
FUZZ_FLAGS := -g -O1 -fno-omit-frame-pointer -fsanitize=fuzzer,address,undefined

fuzz: fuzz_cet_app

fuzz_cet_app: fuzz_cet_app.c
	$(CLANG) $(FUZZ_FLAGS) $< -o $@

fuzz-run: fuzz_cet_app
	./fuzz_cet_app -max_total_time=30 -print_final_stats=1

.PHONY: fuzz fuzz-run

# Otherwise KERNELRELEASE is defined; we've been invoked from the
# kernel build system and can use its language.
else
	obj-m := cet_ioctl.o
endif
