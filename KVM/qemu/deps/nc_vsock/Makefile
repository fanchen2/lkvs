# SPDX-License-Identifier: GPL-2.0-only

BIN := nc-vsock
FUZZ_BIN := fuzz_nc_vsock
CC ?= gcc
CLANG ?= clang
CFLAGS ?= -O2 -Wall -Wextra
FUZZ_FLAGS := -g -O1 -fno-omit-frame-pointer -fsanitize=fuzzer,address,undefined

all: $(BIN)

$(BIN): nc-vsock.c
	$(CC) $(CFLAGS) $^ -o $@

fuzz: $(FUZZ_BIN)

$(FUZZ_BIN): fuzz_nc_vsock.c nc-vsock.c
	$(CLANG) $(FUZZ_FLAGS) fuzz_nc_vsock.c -o $(FUZZ_BIN)

fuzz-run: $(FUZZ_BIN)
	./$(FUZZ_BIN) -max_total_time=30 -print_final_stats=1

clean:
	rm -rf $(BIN) $(FUZZ_BIN)

.PHONY: all fuzz fuzz-run clean
