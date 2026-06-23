// SPDX-License-Identifier: GPL-2.0-only
/*
 * Fuzz harness for cet_app argument parsing logic.
 * Mocks ioctl/open to avoid kernel dependency; exercises the
 * command-line parsing and option dispatch paths.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

/* Mock: intercept open() and ioctl() to avoid real device access */
static int mock_fd = 3;

int __wrap_open(const char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	return mock_fd;
}

int __wrap_ioctl(int fd, unsigned long request, ...)
{
	(void)fd;
	(void)request;
	return 0;
}

int __wrap_close(int fd)
{
	(void)fd;
	return 0;
}

/* Stub sched_setaffinity */
int __wrap_sched_setaffinity(int pid, size_t cpusetsize, const void *mask)
{
	(void)pid;
	(void)cpusetsize;
	(void)mask;
	return 0;
}

/*
 * Re-implement the parsing logic from cet_app.c main() without
 * actually calling the real open/ioctl, to fuzz the argv parsing.
 */
static int fuzz_cet_app_main(int argc, char *argv[])
{
	enum {
		e_shstk1,
		e_xsaves,
		e_ibt1,
		e_ibt2
	} option;

	if (argc == 1) {
		option = e_shstk1;
	} else if (argc == 2) {
		if (strcmp(argv[1], "s1") == 0) {
			option = e_shstk1;
		} else if (strcmp(argv[1], "s2") == 0) {
			option = e_xsaves;
		} else if (strcmp(argv[1], "b1") == 0) {
			option = e_ibt1;
		} else if (strcmp(argv[1], "b2") == 0) {
			option = e_ibt2;
		} else {
			return 1;
		}
	} else {
		return 1;
	}

	(void)option;
	return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char buf[256];
	char *argv[16];
	int argc = 0;
	size_t i, start;
	size_t copy_size;

	if (size == 0)
		return 0;

	copy_size = size < sizeof(buf) - 1 ? size : sizeof(buf) - 1;
	memcpy(buf, data, copy_size);
	buf[copy_size] = '\0';

	/* Split buffer into argv by null bytes or spaces */
	argv[argc++] = "cet_app";
	start = 0;
	for (i = 0; i < copy_size && argc < 15; i++) {
		if (buf[i] == '\0' || buf[i] == ' ' || buf[i] == '\n') {
			buf[i] = '\0';
			if (i > start)
				argv[argc++] = &buf[start];
			start = i + 1;
		}
	}
	if (start < copy_size && argc < 15)
		argv[argc++] = &buf[start];
	argv[argc] = NULL;

	(void)fuzz_cet_app_main(argc, argv);

	return 0;
}
