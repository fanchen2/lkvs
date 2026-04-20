// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 Intel Corporation

#include <stdio.h>
#include <stdlib.h>

#define CPUID_80000006_ECX_CACHE_LINE_SIZE 0x000000FF

static void cpuid(int leaf, int subleaf, int *eax, int *ebx, int *ecx, int *edx)
{
    asm volatile("cpuid;"
                    : "=a" (*eax),
                    "=b" (*ebx),
                    "=c" (*ecx),
                    "=d" (*edx)
                    : "0" (leaf), "2" (subleaf)
                    : "memory");
}

static inline void locked_add_1(int *ptr)
{
    asm volatile("lock; addl $1,%0\n\t"
                    : "+m"(*ptr)
                    :
                    : "memory");
}

static inline int get_cache_line_size(void)
{
    int eax, ecx;

    cpuid(0x80000006, 0, &eax, &eax, &ecx, &eax);
    return ecx & CPUID_80000006_ECX_CACHE_LINE_SIZE;
}

int main(void)
{
    unsigned char *buffer;
    int *int_ptr;
    int cache_line_size;

    cache_line_size = get_cache_line_size();
    printf("The cache line size is %d bytes.\n", cache_line_size);

    buffer = (unsigned char *)aligned_alloc(cache_line_size, 2 * cache_line_size);
    int_ptr = (int *)(buffer + cache_line_size - 1);
    locked_add_1(int_ptr);

    return 0;
}
