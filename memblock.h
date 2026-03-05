#ifndef MY_MEM_BLOCK
#define MY_MEM_BLOCK

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <malloc.h>
#include <string.h>

#ifndef USE_MUTEXES
#define USE_MUTEXES 1
#endif

#if USE_MUTEXES
#include <pthread.h>
#endif

#define MEM_CACHE_LINE_SIZE 64

#define KB_SIZE (1024)
#define MB_SIZE (KB_SIZE * KB_SIZE)

#define ALLOC_BLOCK_SIZE (4 * KB_SIZE)

#define OPTION_ARCH_ALIGNED (0x1)
#define OPTION_CACHE_ALIGNED (0x2)
#define OPTION_USE_MUTEX (0x8)
#define OPTION_USE_GLOBAL_MUTEX (0x10)
#define OPTION_CLEAR_MEM (0x20)

#define OPTION_MASK 0xFF

#define MAX_ITEM_SIZE ((0x1 << 16) - 1)

typedef uint64_t options_t;

#if USE_MUTEXES
pthread_mutex_t allocLock;
#endif
/*
Block structure:
- allocated memory
- pointer to (cache-aligned) real-memory block
-
- block:
- options & size (8 bytes): 1 byte unused, 1 byte options, 2 bytes item size, 2 bytes max items, 2 bytes items used
- (optional) mutex memory
- bitfield of uint64: if bit set the
*/
void *allocBlock(size_t itemSize, options_t opts)
{
    if ((ALLOC_BLOCK_SIZE <= 64 || MEM_CACHE_LINE_SIZE < sizeof(void *)) || itemSize <= 0 || itemSize > MAX_ITEM_SIZE)
    {
        return (void *)0;
    }

    size_t sizeAvail = ALLOC_BLOCK_SIZE;

    void *pMem = malloc(ALLOC_BLOCK_SIZE);

    if (pMem != ((void *)0))
    {
        uint64_t val = (uint64_t)pMem;
        uint64_t offset = MEM_CACHE_LINE_SIZE - ((val) % (MEM_CACHE_LINE_SIZE));

        if (offset < sizeof(void *))
        {
            offset += MEM_CACHE_LINE_SIZE;
        }
        if (offset == MEM_CACHE_LINE_SIZE)
        {
            offset = 0;
        }

        if (sizeAvail <= offset)
        {
            free(pMem);
            return (void *)0;
        }

        void *pBlockStart = ((char *)pMem) + offset;
        void **pRes = (void **)pMem;

        *pRes = pBlockStart;

        size_t headerSize = offset;

        if (offset == 0)
        {
            pBlockStart = ((char *)pBlockStart) + sizeof(void *);
            headerSize = sizeof(void *);
        }

        printf("BLOCK:  mem: %p\n", pBlockStart);
        printf("SIZE:  %ld\n", sizeAvail);

        // header: +options
        uint64_t myOpts = ((opts & 0xFF) << 48);
        // header: +itemSize
        myOpts |= ((itemSize & 0xFFFF) << 32);

        headerSize += sizeof(uint64_t *);

        int error = 0;

        if (sizeAvail > headerSize)
        {
            uint64_t *pOpts = (uint64_t *)pBlockStart;
            void *pDataBlockStart = ((char *)pBlockStart) + sizeof(uint64_t);

#if 0
            if (opts & OPTION_USE_GLOBAL_MUTEX)
            {
                pBlockStart = ((char *)pBlockStart) + sizeof(pthread_mutex_t *);
                headerSize += sizeof(pthread_mutex_t *);
                printf("MTX-PTR-SIZE: %ld\n", sizeof(pthread_mutex_t *));
            }
#endif

            if (opts & OPTION_USE_MUTEX)
            {
                pDataBlockStart = ((char *)pDataBlockStart) + sizeof(pthread_mutex_t);
                headerSize += sizeof(pthread_mutex_t);
                printf("MTX-SIZE: %ld\n", sizeof(pthread_mutex_t));
            }

            if (sizeAvail > headerSize)
            {
                sizeAvail -= headerSize;
                headerSize = 0;
                printf("SIZE:  %ld\n", sizeAvail);

                size_t realItemSize = itemSize;

                if (opts & OPTION_ARCH_ALIGNED)
                {
                    size_t padding = realItemSize % (sizeof(void *));

                    if (padding > 0)
                    {
                        realItemSize += (sizeof(void *)) - padding;
                    }
                }

                if (opts & OPTION_CACHE_ALIGNED)
                {
                    size_t padding = realItemSize % (MEM_CACHE_LINE_SIZE);

                    if (padding > 0)
                    {
                        realItemSize += (MEM_CACHE_LINE_SIZE)-padding;
                    }
                }

                printf("ITEM-SIZE:  %ld <= %ld \n", realItemSize, itemSize);

                // We should be able to manage at least 2 items and a bitfield
                if (realItemSize * 2 + sizeof(uint64_t) <= sizeAvail)
                {
                    size_t numBlocks = 1;
                    size_t perBlock = sizeof(uint64_t) + 64 * realItemSize;

                    if (sizeAvail > perBlock)
                    {
                        numBlocks = (sizeAvail) / perBlock;
                    }
                    else
                    {
                        size_t s = (sizeAvail - sizeof(uint64_t));
                        perBlock = s - (s % realItemSize);
                    }

                    size_t rest = sizeAvail - perBlock * numBlocks;

                    printf("BLOCKS: (%ld) * %ld => %ld (of %ld -= %ld)\n",
                           perBlock, numBlocks, perBlock * numBlocks, sizeAvail, rest);

                    if (rest >= sizeof(uint64_t) + realItemSize)
                    {
                        printf("CAN FIT +%ld\n", (rest - sizeof(uint64_t)) / realItemSize);
                        numBlocks++;
                    }

                    // Nullify usage bits
                    memset(pDataBlockStart, 0, numBlocks * sizeof(uint64_t));

                    size_t all = (sizeAvail - (numBlocks * sizeof(uint64_t))) / realItemSize;

                    // header: +all items
                    myOpts |= (all << 16);

                    *pOpts = myOpts;
                }
                else
                {
                    error = 3;
                }
            }
            else
            {
                error = 2;
            }
        }
        else
        {
            error = 1;
        }

        if (error != 0)
        {
            free(pMem);
            pMem = (void *)0;
        }
    }
    return pMem;
}

void blockInfo(void *pBlock)
{
    if (pBlock != ((void *)0))
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

        d = ((opts2 & OPTION_USE_GLOBAL_MUTEX) != 0) ? 1 : 0;
        printf("MUTEX:global: %d\n", d);

        d = ((opts2 & OPTION_USE_MUTEX) != 0) ? 1 : 0;
        printf("MUTEX:single: %d\n", d);

        d = ((opts2 & OPTION_ARCH_ALIGNED) != 0) ? 1 : 0;
        printf("ALIGN:arch: %d\n", d);

        d = ((opts2 & OPTION_CACHE_ALIGNED) != 0) ? 1 : 0;
        printf("ALIGN:cache: %d\n", d);
        puts("---\n");
    }
}

void *allocItem(void *pBlock, void *pInit)
{
    if (pBlock != (void *)0)
    {
        void *pBlockStart = (*((void **)pBlock));

        if (pBlockStart == pBlock)
        {
            pBlockStart = (char *)pBlockStart + sizeof(void *);
        }

        options_t opts = *(uint64_t *)pBlockStart;
        options_t opts2 = opts >> 48;

        void *pDataBlockStart = (char *)pBlockStart + sizeof(uint64_t);

#if USE_MUTEXES
        pthread_mutex_t *pMutex = (pthread_mutex_t *)0;

        if (opts2 & OPTION_USE_GLOBAL_MUTEX)
        {
            pthread_mutex_lock(&allocLock);
        }

        if (opts2 & OPTION_USE_MUTEX)
        {
            pMutex = (pthread_mutex_t *)pDataBlockStart;
            pDataBlockStart = (char *)pDataBlockStart + sizeof(pthread_mutex_t);

            pthread_mutex_lock(pMutex);
        }
#endif
        // Need to read again to have actual info about used items
        opts = *(uint64_t *)pBlockStart;

        size_t used = (opts) & 0xFFFFFFFF;
        size_t all = used >> 16;
        used &= 0xFFFF;

        void *pRes = (void *)0;

        if (used < all)
        {
            uint64_t *pMemBits = (uint64_t *)pDataBlockStart;

            // Search bit field
            size_t i = 0, numBlocks = (all + 63) >> 6;

            // Skip fully used slots
            for (; i < numBlocks && pMemBits[i] == ~((uint64_t)0); i++)
            {
            }

            if (i < numBlocks)
            {
                size_t itemIdx = i << 6;
                uint64_t bits = pMemBits[i];

                while (bits & 0x1)
                {
                    itemIdx++;
                    bits >>= 1;
                }

                size_t itemSize = (opts >> 32) & 0xFFFF;
                size_t realItemSize = itemSize;

                if (opts & OPTION_ARCH_ALIGNED)
                {
                    size_t padding = realItemSize % (sizeof(void *));
                    if (padding > 0)
                    {
                        realItemSize += (sizeof(void *)) - padding;
                    }
                }

                if (opts & OPTION_CACHE_ALIGNED)
                {
                    size_t padding = realItemSize % (MEM_CACHE_LINE_SIZE);
                    if (padding > 0)
                    {
                        realItemSize += (MEM_CACHE_LINE_SIZE)-padding;
                    }
                }

                used++;

                opts &= ~((uint64_t)0xFFFF);
                opts |= used;

                *(uint64_t *)pBlockStart = opts;

                bits = 0x1 << (itemIdx & 0x3F);
                pMemBits[i] |= bits;

                size_t offset = (numBlocks * sizeof(uint64_t)) + (itemIdx * realItemSize);

                pRes = (char *)pDataBlockStart + offset;

                if (pInit != 0)
                {
                    char *pResChar = (char *)pRes;
                    char *pInitChar = (char *)pInit;
                    size_t i = 0;
                    for (; i < itemSize; i++)
                    {
                        *pResChar = *pInitChar;
                        ++pResChar;
                        ++pInitChar;
                    }
                    if (opts & OPTION_CLEAR_MEM)
                    {
                        for (; i < realItemSize; i++)
                        {
                            *pInitChar = 0;
                            ++pInitChar;
                        }
                    }
                }
                else
                {
                    if (opts & OPTION_CLEAR_MEM)
                    {
                        memset(pRes, 0, realItemSize);
                    }
                }
            }
        }

#if USE_MUTEXES
        if (pMutex)
        {
            pthread_mutex_unlock(pMutex);
        }

        if (opts2 & OPTION_USE_GLOBAL_MUTEX)
        {
            pthread_mutex_unlock(&allocLock);
        }
#endif
        return pRes;
    }

    return (void *)0;
}

int freeItem(void *pBlock, void *pItem)
{
    if ((pBlock != (void *)0) && (pItem != (void *)0))
    {
#if USE_MUTEXES
        pthread_mutex_lock(&allocLock);
#endif
        // Find offset bit and unset it
        // Clear if needed

#if USE_MUTEXES
        pthread_mutex_unlock(&allocLock);
#endif
    }
    return 1;
}

void freeBlock(void *pBlock)
{
#if USE_MUTEXES
    pthread_mutex_lock(&allocLock);
#endif

    if (pBlock != ((void *)0))
    {
        free(pBlock);
    }

#if USE_MUTEXES
    pthread_mutex_unlock(&allocLock);
#endif
}

#endif