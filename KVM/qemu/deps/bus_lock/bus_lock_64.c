#include <stdio.h>
#include <stdlib.h>

static inline void locked_add_1(int * ptr)
{
    asm volatile(   "lock; addl $1,%0\n\t"
                    : "+m"(*ptr)
                    :
                    : "memory"
                    );
}

static inline int get_cache_line_size(void)
{
    return 64;
}

int main(void)
{
    unsigned char * buffer;
    int * int_ptr;
    int cache_line_size;
    int idx;

    cache_line_size = get_cache_line_size();
    printf("The cache line size is %d bytes.\n", cache_line_size);

    /* allocate a cache_line_size aligned memory */
    buffer = (unsigned char *) aligned_alloc(cache_line_size, 2 * cache_line_size);

    /*
     * Increment the pointer by cache_line_size - 1, making it 4-byte int_ptr that
     * across two cache line
     */
    int_ptr = (int *) (buffer + cache_line_size - 1);

    for (idx = 0; idx < 10; idx++)
        locked_add_1(int_ptr);

    return 0;
}
