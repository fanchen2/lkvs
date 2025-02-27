# SPDX-License-Identifier: GPL-2.0-only

MODULES = tdx-compliance.ko

ifeq ($(KERNEL_SRC),)
KDIR ?= /lib/modules/$(shell uname -r)/build
else
KDIR ?= $(abspath $(KERNEL_SRC))
endif

obj-m += tdx-compliance.o
tdx-compliance-y := tdx-compliance.o tdcall.o tdxcall.o

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	@make -C $(KDIR) M=$(PWD) clean


