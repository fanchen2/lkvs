// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 Intel Corporation.

#include <stddef.h>
#include <stdint.h>

extern unsigned int extract_bits(unsigned int num, int start, int end);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	unsigned int num;
	int start;
	int end;
	unsigned int out;

	if (size < 6)
		return 0;

	num = ((unsigned int)data[0] << 24) |
	      ((unsigned int)data[1] << 16) |
	      ((unsigned int)data[2] << 8) |
	      (unsigned int)data[3];

	/*
	 * Keep a broad range so the fuzzer can exercise edge-case shifts,
	 * while avoiding extreme UB from very large negative values.
	 */
	start = (int)data[4] - 16;
	end = (int)data[5] - 16;

	out = extract_bits(num, start, end);

	/* Prevent optimizer from removing the call. */
	(void)out;

	return 0;
}
