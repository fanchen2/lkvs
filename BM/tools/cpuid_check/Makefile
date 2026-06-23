# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023 Intel Corporation.

BIN := cpuid_check
FUZZ_BIN := fuzz_cpuid_check
CLANG ?= clang
FUZZ_FLAGS := -g -O1 -fno-omit-frame-pointer -fsanitize=fuzzer,address,undefined

all: $(BIN)

install:
	install cpuid_check /usr/local/bin/

cpuid_check: cpuid_check.c
	gcc $^ -o $@

fuzz: $(FUZZ_BIN)

$(FUZZ_BIN): fuzz_cpuid_check.c cpuid_check.c
	$(CLANG) $(FUZZ_FLAGS) -Dmain=cpuid_check_main -c cpuid_check.c -o cpuid_check.fuzz.o
	$(CLANG) $(FUZZ_FLAGS) cpuid_check.fuzz.o fuzz_cpuid_check.c -o $(FUZZ_BIN)

fuzz-run: $(FUZZ_BIN)
	./$(FUZZ_BIN) -max_total_time=30 -print_final_stats=1

clean:
	rm -rf $(BIN) $(FUZZ_BIN) *.o *.profraw *.profdata

.PHONY: all install fuzz fuzz-run clean
