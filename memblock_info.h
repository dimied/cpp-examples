#ifndef MY_MEM_BLOCK_INFO
#define MY_MEM_BLOCK_INFO

#ifndef MEMBLOCK_NULLPTR
#error "memblock not included"
#endif

#include <stdio.h>

/**
 * @param pBlock Pointer to memory block allocated with allocBlock().
 */
void blockInfo(void *pBlock)
{
    if (pBlock != MEMBLOCK_NULLPTR)
    {
        void *pBlockStart = (*((void **)pBlock));
        int d = (pBlockStart == pBlock) ? 1 : 0;
        puts("\n---");
        printf("BLOCK-START:same: %d (%p <- %p)\n", d, pBlockStart, pBlock);

        if (d == 1)
        {
            pBlockStart = (char *)pBlockStart + sizeof(void *);
        }
        uint64_t opts = *((uint64_t *)pBlockStart);
        uint64_t opts2 = opts >> 48;

        size_t itemSize = (opts >> 32) & 0xFFFF;
        printf("ITEM-SIZE: %ld\n", itemSize);
        // printf("OPTS: %b\n", opts2);

        d = ((opts2 & MEMBLOCK_OPTION_ARCH_ALIGNED) != 0) ? 1 : 0;
        printf("ALIGN:arch: %d\n", d);

        d = ((opts2 & MEMBLOCK_OPTION_CACHE_ALIGNED) != 0) ? 1 : 0;
        printf("ALIGN:cache: %d\n", d);

        d = ((opts2 & MEMBLOCK_OPTION_HAS_PTR) != 0) ? 1 : 0;
        printf("PTR: %d\n", d);

        d = ((opts2 & MEMBLOCK_OPTION_USE_GLOBAL_MUTEX) != 0) ? 1 : 0;
        printf("MUTEX:global: %d\n", d);

        d = ((opts2 & MEMBLOCK_OPTION_USE_MUTEX) != 0) ? 1 : 0;
        printf("MUTEX:single: %d\n", d);
        puts("---");
    }
}


#endif