// SPDX-License-Identifier: GPL-2.0-only

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define main nc_vsock_main
#include "nc-vsock.c"
#undef main

static size_t bounded_copy(char *dst, size_t dst_size, const uint8_t *src, size_t src_size)
{
	size_t n = src_size;

	if (dst_size == 0)
		return 0;
	if (n >= dst_size)
		n = dst_size - 1;
	memcpy(dst, src, n);
	dst[n] = '\0';
	return n;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char cid[128];
	char port[128];
	size_t split;

	if (size == 0)
		return 0;

	split = data[0] % size;
	bounded_copy(cid, sizeof(cid), data, split);
	bounded_copy(port, sizeof(port), data + split, size - split);

	(void)parse_cid(cid);
	(void)parse_port(port);
	(void)parse_cid(port);
	(void)parse_port(cid);

	return 0;
}
