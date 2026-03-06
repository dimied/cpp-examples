#ifndef MY_MEM_BLOCK
#define MY_MEM_BLOCK

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef USE_MUTEXES
#define USE_MUTEXES 1
#endif

#if USE_MUTEXES
#include <pthread.h>
#endif

#ifndef MEMBLOCK_ALWAYS_SET_ERROR
#define MEMBLOCK_ALWAYS_SET_ERROR 0
#endif

#define MEM_SIZE_KB (1 << 10)
#define MEM_SIZE_MB (1 << 20)

#define MEMBLOCK_CACHE_LINE_SIZE 64

#define MEMBLOCK_ALLOC_BLOCK_SIZE (1 << 12) //

// Reserve bytes by malloc
#define MEMBLOCK_MALLOC_RESERVE (16)

#define MEMBLOCK_SET_BLOCK_SIZE(OPTS, SIZE) ((OPTS) | (SIZE << 8))
#define MEMBLOCK_GET_BLOCK_SIZE(OPTS) ((OPTS) >> 8)

#define MEMBLOCK_OPTION_ARCH_ALIGNED (0x1)
#define MEMBLOCK_OPTION_CACHE_ALIGNED (0x2)
#define MEMBLOCK_OPTION_CLEAR_MEM (0x4)

#define MEMBLOCK_OPTION_HAS_PTR (0x10)
#define MEMBLOCK_OPTION_USE_MUTEX (0x20)
#define MEMBLOCK_OPTION_USE_GLOBAL_MUTEX (0x40)

#define MEMBLOCK_OPTION_MASK 0xFF

#define MEMBLOCK_MAX_ITEM_SIZE ((0x1 << 16) - 1)

#ifdef NULL
#define MEMBLOCK_NULLPTR NULL
#else
#define MEMBLOCK_NULLPTR ((void *)*)
#endif

typedef uint64_t options_t;

#if USE_MUTEXES
pthread_mutex_t allocLock;
#endif

int memblockError = 0;

#define MEMBLOCK_INTERNAL_VOIDPTR_SIZE (sizeof(void *))
#define MEMBLOCK_INTERNAL_OPTIONS_SIZE (sizeof(uint64_t))

#define MEMBLOCK_INTERNAL_ITEMSIZE_SHIFT (32)
#define MEMBLOCK_INTERNAL_OPTIONS_SHIFT (48)
#define MEMBLOCK_INTERNAL_NUM_MEMBLOCKS_SHIFT (56)

// MACROS:
#define CALC_REAL_ITEM_SIZE(REAL_ITEM_SZ, OPTS)                     \
    if (OPTS & MEMBLOCK_OPTION_ARCH_ALIGNED)                        \
    {                                                               \
        size_t padding = REAL_ITEM_SZ % (sizeof(void *));           \
        if (padding > 0)                                            \
        {                                                           \
            REAL_ITEM_SZ += (sizeof(void *)) - padding;             \
        }                                                           \
    }                                                               \
    if (OPTS & MEMBLOCK_OPTION_CACHE_ALIGNED)                       \
    {                                                               \
        size_t padding = REAL_ITEM_SZ % (MEMBLOCK_CACHE_LINE_SIZE); \
        if (padding > 0)                                            \
        {                                                           \
            REAL_ITEM_SZ += (MEMBLOCK_CACHE_LINE_SIZE) - padding;   \
        }                                                           \
    }

#define MEMBLOCK_ERROR_NOT_ENOUGH 1
#define MEMBLOCK_ERROR_ITEMSIZE 2
#define MEMBLOCK_ERROR_BLOCKSIZE_FAIL 3
#define MEMBLOCK_ERROR_MIN_ITEMS_FAIL 4
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
void *allocBlock(size_t itemSize, const options_t opts, int *pError)
{
    // no support for 16-bit systems
    assert(sizeof(void *) >= 4);
    assert(sizeof(uint64_t) >= sizeof(void *));
    assert(MEMBLOCK_ALLOC_BLOCK_SIZE > 64);
    assert(MEMBLOCK_CACHE_LINE_SIZE >= sizeof(void *));

    if (itemSize <= 0 || itemSize > MEMBLOCK_MAX_ITEM_SIZE)
    {
        if (pError == MEMBLOCK_NULLPTR)
        {
            pError = &memblockError;
        }
        *pError = MEMBLOCK_ERROR_ITEMSIZE;

        return MEMBLOCK_NULLPTR;
    }

    // 0 and 1 means the same: at least one memory block
    size_t numMemBlocks = MEMBLOCK_GET_BLOCK_SIZE(opts);

    if (numMemBlocks > 255)
    {
        numMemBlocks = 256;
    }

    size_t sizeAvail = MEMBLOCK_ALLOC_BLOCK_SIZE;

    if (numMemBlocks > 0)
    {
        sizeAvail = numMemBlocks * MEMBLOCK_ALLOC_BLOCK_SIZE;
        // 0 means 1, see above
        --numMemBlocks;
    }

    int error = 0;
    // Do we expect malloc to use some of allocate memory for its needs?
#if (MEMBLOCK_MALLOC_RESERVE > 0)
    // If so, check it ...
    if (MEMBLOCK_MALLOC_RESERVE >= sizeAvail)
    {
        error = MEMBLOCK_ERROR_BLOCKSIZE_FAIL;
        goto error_point;
        // return MEMBLOCK_NULLPTR;
    }
    // ... adapt size
    sizeAvail -= MEMBLOCK_MALLOC_RESERVE;
#endif

    // Make some estimations.
    //- Does it 'make sense' to allocate memory at all?
    //
    //+options field
    size_t headerSize = sizeof(uint64_t);

    // Reserve place for some pointer?
    if (opts & MEMBLOCK_OPTION_HAS_PTR)
    {
        headerSize += sizeof(void *);
    }

    // Store mutex?
    if (opts & MEMBLOCK_OPTION_USE_MUTEX)
    {
        headerSize += sizeof(pthread_mutex_t);
    }

    size_t realItemSize = itemSize;

    CALC_REAL_ITEM_SIZE(realItemSize, opts)

#define MAX_OFFSET (MEMBLOCK_CACHE_LINE_SIZE + sizeof(void *) - 1)

#define MEMBLOCK_MIN_NUM_ITEMS 2
#define MEMBLOCK_MIN_NUM_BITFIELDS ((MEMBLOCK_MIN_NUM_ITEMS + 63) >> 6)

    {
        const size_t minRequired = sizeof(uint64_t) * MEMBLOCK_MIN_NUM_BITFIELDS + MAX_OFFSET + MEMBLOCK_MIN_NUM_ITEMS * realItemSize;

        if (sizeAvail < headerSize + minRequired)
        {
            error = MEMBLOCK_ERROR_MIN_ITEMS_FAIL;
            goto error_point;
        }
    }

    void *const pMem = malloc(sizeAvail);

    // TODO: try smaller sizes if allocation fails

    if (pMem == MEMBLOCK_NULLPTR)
    {
        error = MEMBLOCK_ERROR_NOT_ENOUGH;
        goto error_point;
        // return pMem;
    }

    const uint64_t val = (uint64_t)pMem;
    uint64_t offset = (val % (MEMBLOCK_CACHE_LINE_SIZE));

    if (offset > 0)
    {
        offset = MEMBLOCK_CACHE_LINE_SIZE - offset;

        if (offset < MEMBLOCK_INTERNAL_VOIDPTR_SIZE)
        {
            offset += MEMBLOCK_CACHE_LINE_SIZE;
        }
    }
    else
    {
        offset = MEMBLOCK_INTERNAL_VOIDPTR_SIZE;
    }

    void *const pBlockStart = ((char *)pMem) + offset;
    {
        void **pRes = (void **)pMem;
        // Set pointer to header start
        *pRes = pBlockStart;
    }

    sizeAvail -= offset;
    sizeAvail -= headerSize;

    size_t numBlocks = 1, bytesPerBlock = sizeof(uint64_t) + 64 * realItemSize;

    if (sizeAvail >= bytesPerBlock)
    {
        numBlocks = sizeAvail / bytesPerBlock;
        bytesPerBlock -= 8;
    }
    else
    {
        size_t s = (sizeAvail - sizeof(uint64_t));
        bytesPerBlock = s - (s % realItemSize);
    }

    {
        const size_t rest = sizeAvail - (bytesPerBlock + 8) * numBlocks;

        // Can we fit some additional item and usage bits ?
        if (rest >= sizeof(uint64_t) + realItemSize)
        {
            numBlocks++;
        }
    }

    const size_t usageBitfieldSize = numBlocks * sizeof(uint64_t);
    const size_t all = (sizeAvail - usageBitfieldSize) / realItemSize;

    // header: +options
    uint64_t myOpts = ((opts & 0xFF) << MEMBLOCK_INTERNAL_OPTIONS_SHIFT);
    // header: +#blocks
    myOpts |= ((numMemBlocks & 0xFF) << MEMBLOCK_INTERNAL_ITEMSIZE_SHIFT);
    // header: +itemSize
    myOpts |= ((itemSize & 0xFFFF) << MEMBLOCK_INTERNAL_ITEMSIZE_SHIFT);

    // header: +all items
    myOpts |= (all << 16);

    memset(pBlockStart, 0, headerSize);

    *((uint64_t *)pBlockStart) = myOpts;

    void *const pDataBlockStart = ((char *)pBlockStart) + headerSize;

    // Nullify usage bits
    memset(pDataBlockStart, 0, usageBitfieldSize);

error_point:

#if MEMBLOCK_ALWAYS_SET_ERROR
    if ((pMem != MEMBLOCK_NULLPTR) && error != 0)
    {
        free(pMem);
    }
    if (pError == MEMBLOCK_NULLPTR)
    {
        pError = &memblockError;
    }
    *pError = error;
    if (error != 0)
    {
        return MEMBLOCK_NULLPTR;
    }
#else
    if (error != 0)
    {
        if (pMem != MEMBLOCK_NULLPTR)
        {
            free(pMem);
        }
        if (pError == MEMBLOCK_NULLPTR)
        {
            pError = &memblockError;
        }
        *pError = error;

        return MEMBLOCK_NULLPTR;
    }
    else if (pError != MEMBLOCK_NULLPTR)
    {
        *pError = error;
    }
#endif

    return pMem;
}

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
        puts("---\n");
    }
}

/**
 *
 */
void *allocItem(void *pBlock, void *pInit)
{
    if (pBlock == MEMBLOCK_NULLPTR)
    {
        return MEMBLOCK_NULLPTR;
    }
    void *const pBlockStart = (*((void **)pBlock));

    options_t opts = *(uint64_t *)pBlockStart;
    options_t opts2 = opts >> 48;

    char *pDataBlockStart = (char *)pBlockStart + sizeof(uint64_t);

#if USE_MUTEXES
    pthread_mutex_t *pMutex = (pthread_mutex_t *)0;

    if (opts2 & MEMBLOCK_OPTION_USE_GLOBAL_MUTEX)
    {
        pthread_mutex_lock(&allocLock);
    }

    if (opts2 & MEMBLOCK_OPTION_HAS_PTR)
    {
        pDataBlockStart = (char *)pDataBlockStart + MEMBLOCK_INTERNAL_VOIDPTR_SIZE;
    }

    if (opts2 & MEMBLOCK_OPTION_USE_MUTEX)
    {
        pMutex = (pthread_mutex_t *)pDataBlockStart;
        pDataBlockStart = (char *)pDataBlockStart + sizeof(pthread_mutex_t);

        pthread_mutex_lock(pMutex);
    }
#endif
    // Need to read again to have actual info about used items
    opts = *(uint64_t *)pBlockStart;

    size_t used = (opts) & 0xFFFFFFFF;
    const size_t all = used >> 16;
    used &= 0xFFFF;

    void *pRes = (void *)0;

    used++;

    if (used <= all)
    {
        uint64_t *pMemBits = (uint64_t *)pDataBlockStart;

        // Search bit field
        size_t i = 0, numBlocks = (all + 63) >> 6;

#define MEMBLOCK_INTERNAL_ALLBITS_USED (~((uint64_t)0))

        // Skip fully used slots
        for (; i < numBlocks && pMemBits[i] == MEMBLOCK_INTERNAL_ALLBITS_USED; i++)
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

            if (itemIdx < (i + 1) << 6)
            {
                size_t itemSize = (opts >> 32) & 0xFFFF;
                size_t realItemSize = itemSize;

                CALC_REAL_ITEM_SIZE(realItemSize, opts)

                opts &= ~((uint64_t)0xFFFF);
                opts |= used;

                *(uint64_t *)pBlockStart = opts;

                bits = 0x1 << (itemIdx & 0x3F);
                pMemBits[i] |= bits;

                const size_t offset = (numBlocks * sizeof(uint64_t)) + (itemIdx * realItemSize);

                pRes = (char *)pDataBlockStart + offset;

                if (pInit != 0)
                {
                    char *pResChar = (char *)pRes;

                    size_t i = 0;

                    if (itemSize <= 32)
                    {
                        char *pInitChar = (char *)pInit;

                        for (; i < itemSize; i++)
                        {
                            *pResChar = *pInitChar;
                            ++pResChar;
                            ++pInitChar;
                        }
                    }
                    else
                    {
                        memcpy(pRes, pInit, itemSize);
                        i = itemSize;
                    }

                    if (opts & MEMBLOCK_OPTION_CLEAR_MEM)
                    {
                        for (; i < realItemSize; i++)
                        {
                            *pResChar = 0;
                            ++pResChar;
                        }
                    }
                }
                else
                {
                    if (opts & MEMBLOCK_OPTION_CLEAR_MEM)
                    {
                        memset(pRes, 0, realItemSize);
                    }
                }
            }
        }
    }

#if USE_MUTEXES
    if (pMutex)
    {
        pthread_mutex_unlock(pMutex);
    }

    if (opts2 & MEMBLOCK_OPTION_USE_GLOBAL_MUTEX)
    {
        pthread_mutex_unlock(&allocLock);
    }
#endif
    return pRes;
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