// SPDX-License-Identifier: GPL-2.0-only

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Fuzz target for pcie_check string parsing and hex format handling.
 * Focuses on sscanf and hex conversion edge cases.
 */

static int parse_hex_string(const char *str, unsigned long *val)
{
	char *end = NULL;

	if (!str || *str == '\0')
		return -1;

	*val = strtoul(str, &end, 16);
	if (*end != '\0' && *end != '-' && *end != ':')
		return -1;

	return 0;
}

static int parse_range_hex(const char *str, unsigned long *start, unsigned long *end)
{
	int ret;
	char buf[128];
	char *dash;

	if (!str || strlen(str) >= sizeof(buf))
		return -1;

	strncpy(buf, str, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	dash = strchr(buf, '-');
	if (!dash)
		return parse_hex_string(buf, start);

	*dash = '\0';
	ret = parse_hex_string(buf, start);
	if (ret)
		return ret;
	return parse_hex_string(dash + 1, end);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char buf[256];
	unsigned long val1, val2;
	size_t copy_size;

	if (size == 0)
		return 0;

	copy_size = size < sizeof(buf) - 1 ? size : sizeof(buf) - 1;
	memcpy(buf, data, copy_size);
	buf[copy_size] = '\0';

	(void)parse_hex_string(buf, &val1);
	(void)parse_range_hex(buf, &val1, &val2);

	return 0;
}
