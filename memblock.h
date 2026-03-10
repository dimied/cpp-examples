#ifndef MY_MEM_BLOCK
#define MY_MEM_BLOCK

#ifndef MEMBLOCK_USE_MUTEXES
#define MEMBLOCK_USE_MUTEXES 1
#endif

#ifndef MEMBLOCK_USE_ASSERTS
#define MEMBLOCK_USE_ASSERTS 1
#endif

#if MEMBLOCK_IMPLEMENTATION

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#if MEMBLOCK_USE_MUTEXES
#include <pthread.h>
#endif

#if MEMBLOCK_USE_ASSERTS
#include <assert.h>
#endif

#ifndef MEMBLOCK_DECLARATION
#define MEMBLOCK_DECLARATION 1
#else
#if !(MEMBLOCK_DECLARATION)
#undef MEMBLOCK_DECLARATION
#define MEMBLOCK_DECLARATION 1
#endif
#endif

#endif

#if MEMBLOCK_DECLARATION

// Reserve bytes by malloc
#ifndef MEMBLOCK_MALLOC_RESERVE
#define MEMBLOCK_MALLOC_RESERVE (16)
#endif

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

#define MEMBLOCK_ERROR_NO_ERROR 0
#define MEMBLOCK_RESULT_SUCCESS 0
#define MEMBLOCK_ERROR_NOT_ENOUGH 1
#define MEMBLOCK_ERROR_ITEMSIZE 2
#define MEMBLOCK_ERROR_BLOCKSIZE_FAIL 3
#define MEMBLOCK_ERROR_MIN_ITEMS_FAIL 4

#define MEMBLOCK_PTR_TO_DEFAULT_MUTEX_ATTRS (NULL)

void memblockInit();
void memblockDestroy();

void *allocBlock(size_t itemSize, const options_t opts, int *pError);
void freeBlock(void *pBlock);

void *allocItem(void *pBlock, void *pInit);
int freeItem(void *pBlock, void *pItem);

void *getStoredPointer(void *pBlock);
int setStoredPointer(void *pBlock, void *ptrToStore);

#endif

#ifndef MEMBLOCK_ALWAYS_SET_ERROR
#define MEMBLOCK_ALWAYS_SET_ERROR 0
#endif

#ifndef MEMBLOCK_malloc
#define MEMBLOCK_malloc(SZ) (malloc(SZ))
#define MEMBLOCK_free(PTR) (free(PTR))
#endif

#ifndef MEMBLOCK_CACHE_LINE_SIZE
#define MEMBLOCK_CACHE_LINE_SIZE 64
#endif

#ifndef MEMBLOCK_ALLOC_BLOCK_SIZE
#define MEMBLOCK_ALLOC_BLOCK_SIZE (1 << 12)
#endif

#if MEMBLOCK_IMPLEMENTATION

int memblockError = 0;
#if MEMBLOCK_USE_MUTEXES
pthread_mutex_t allocLock = PTHREAD_MUTEX_INITIALIZER;
#endif

#define MEMBLOCK_INTERNAL_VOIDPTR_SIZE (sizeof(void *))
#define MEMBLOCK_INTERNAL_OPTIONS_SIZE (sizeof(uint64_t))

#define MEMBLOCK_INTERNAL_ITEMSIZE_SHIFT (32)
#define MEMBLOCK_INTERNAL_OPTIONS_SHIFT (48)
#define MEMBLOCK_INTERNAL_NUM_MEMBLOCKS_SHIFT (56)

#define MEMBLOCK_INTERNAL_OPTION_ARCH_ALIGNED (((uint64_t)MEMBLOCK_OPTION_ARCH_ALIGNED) << MEMBLOCK_INTERNAL_OPTIONS_SHIFT)
#define MEMBLOCK_INTERNAL_OPTION_CACHE_ALIGNED (((uint64_t)MEMBLOCK_OPTION_CACHE_ALIGNED) << MEMBLOCK_INTERNAL_OPTIONS_SHIFT)
#define MEMBLOCK_INTERNAL_OPTION_CLEAR_MEM (((uint64_t)MEMBLOCK_OPTION_CLEAR_MEM) << MEMBLOCK_INTERNAL_OPTIONS_SHIFT)

#define MEMBLOCK_INTERNAL_OPTION_HAS_PTR (((uint64_t)MEMBLOCK_OPTION_HAS_PTR) << MEMBLOCK_INTERNAL_OPTIONS_SHIFT)
#define MEMBLOCK_INTERNAL_OPTION_USE_MUTEX (((uint64_t)MEMBLOCK_OPTION_USE_MUTEX) << MEMBLOCK_INTERNAL_OPTIONS_SHIFT)
#define MEMBLOCK_INTERNAL_OPTION_USE_GLOBAL_MUTEX (((uint64_t)MEMBLOCK_OPTION_USE_GLOBAL_MUTEX) << MEMBLOCK_INTERNAL_OPTIONS_SHIFT)

// MACROS:
#define MEMBLOCK_CALC_REAL_ITEM_SIZE(REAL_ITEM_SZ, OPTS)            \
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

#define MEMBLOCK_INTERNAL_CALC_REAL_ITEM_SIZE(REAL_ITEM_SZ, OPTS)   \
    if (OPTS & MEMBLOCK_INTERNAL_OPTION_ARCH_ALIGNED)               \
    {                                                               \
        size_t padding = REAL_ITEM_SZ % (sizeof(void *));           \
        if (padding > 0)                                            \
        {                                                           \
            REAL_ITEM_SZ += (sizeof(void *)) - padding;             \
        }                                                           \
    }                                                               \
    if (OPTS & MEMBLOCK_INTERNAL_OPTION_CACHE_ALIGNED)              \
    {                                                               \
        size_t padding = REAL_ITEM_SZ % (MEMBLOCK_CACHE_LINE_SIZE); \
        if (padding > 0)                                            \
        {                                                           \
            REAL_ITEM_SZ += (MEMBLOCK_CACHE_LINE_SIZE) - padding;   \
        }                                                           \
    }

void memblockInit()
{
#if MEMBLOCK_USE_MUTEXES
    pthread_mutex_init(&allocLock, MEMBLOCK_PTR_TO_DEFAULT_MUTEX_ATTRS);
#endif
}

void memblockDestroy()
{
#if MEMBLOCK_USE_MUTEXES
    pthread_mutex_destroy(&allocLock);
#endif
}
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
#if MEMBLOCK_USE_ASSERTS
    // no support for 16-bit systems
    assert(sizeof(void *) >= 4);
    assert(sizeof(void *) == sizeof(char *));
    assert(sizeof(void *) == sizeof(uint64_t *));
    assert(sizeof(uint64_t) >= sizeof(void *));
    assert(MEMBLOCK_CACHE_LINE_SIZE >= sizeof(void *));
    assert(MEMBLOCK_CACHE_LINE_SIZE % sizeof(void *) == 0);
    assert(MEMBLOCK_ALLOC_BLOCK_SIZE >= (2 * MEMBLOCK_CACHE_LINE_SIZE));
    assert(MEMBLOCK_ALLOC_BLOCK_SIZE % MEMBLOCK_CACHE_LINE_SIZE == 0);
    assert(MEMBLOCK_ALLOC_BLOCK_SIZE <= (0x1 << 26));
#endif

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
        // we decrement, such that we can allocate up to 256 memory blocks at once
        --numMemBlocks;
    }

    // Do we expect malloc to use some of allocate memory for its needs?
#if (MEMBLOCK_MALLOC_RESERVE > 0)
    // If so, check it ...
    if (MEMBLOCK_MALLOC_RESERVE >= sizeAvail)
    {
        if (pError == MEMBLOCK_NULLPTR)
        {
            pError = &memblockError;
        }
        *pError = MEMBLOCK_ERROR_BLOCKSIZE_FAIL;

        return MEMBLOCK_NULLPTR;
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

#if MEMBLOCK_USE_MUTEXES
    const size_t mutexOffset = headerSize;
    // Store mutex?
    if (opts & MEMBLOCK_OPTION_USE_MUTEX)
    {
        headerSize += sizeof(pthread_mutex_t);
    }

#endif

    size_t realItemSize = itemSize;

    MEMBLOCK_CALC_REAL_ITEM_SIZE(realItemSize, opts)

#define MAX_OFFSET (MEMBLOCK_CACHE_LINE_SIZE + sizeof(void *) - 1)

#define MEMBLOCK_MIN_NUM_ITEMS 2
#define MEMBLOCK_MIN_NUM_BITFIELDS ((MEMBLOCK_MIN_NUM_ITEMS + 63) >> 6)

    {
        const size_t minRequired = sizeof(uint64_t) * MEMBLOCK_MIN_NUM_BITFIELDS + MAX_OFFSET + MEMBLOCK_MIN_NUM_ITEMS * realItemSize;

        if (sizeAvail < headerSize + minRequired)
        {
            if (pError == MEMBLOCK_NULLPTR)
            {
                pError = &memblockError;
            }
            *pError = MEMBLOCK_ERROR_MIN_ITEMS_FAIL;
            return MEMBLOCK_NULLPTR;
        }
    }

    void *const pMem = MEMBLOCK_malloc(sizeAvail);

    // TODO: try smaller sizes if allocation fails

    if (pMem == MEMBLOCK_NULLPTR)
    {
        if (pError == MEMBLOCK_NULLPTR)
        {
            pError = &memblockError;
        }
        *pError = MEMBLOCK_ERROR_NOT_ENOUGH;

        return MEMBLOCK_NULLPTR;
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
    size_t all = (sizeAvail - usageBitfieldSize) / realItemSize;

    if (all > 0xFFFF)
    {
        // Here we actually waste space!
        all = 0xFFFF;
    }

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

#if MEMBLOCK_USE_MUTEXES
    // Store mutex?
    if (opts & MEMBLOCK_OPTION_USE_MUTEX)
    {
        pthread_mutex_t *pMutex = (pthread_mutex_t *)((char *)pBlockStart + mutexOffset);
        pthread_mutex_init(pMutex, MEMBLOCK_PTR_TO_DEFAULT_MUTEX_ATTRS);
    }
#endif

    void *const pDataBlockStart = ((char *)pBlockStart) + headerSize;

    // Nullify usage bits
    memset(pDataBlockStart, 0, usageBitfieldSize);

#if MEMBLOCK_ALWAYS_SET_ERROR
    if (pError == MEMBLOCK_NULLPTR)
    {
        pError = &memblockError;
    }
    *pError = error;
#endif

    return pMem;
}

/**
 * @param pBlock
 * @param pInit
 * @returns
 */
void *allocItem(void *pBlock, void *pInit)
{
    if (pBlock == MEMBLOCK_NULLPTR)
    {
        return MEMBLOCK_NULLPTR;
    }

    void *const pBlockStart = (*((void **)pBlock));

    options_t opts = *(uint64_t *)pBlockStart;
    // options_t
    opts = opts >> 48;

    char *pDataBlockStart = (char *)pBlockStart + sizeof(uint64_t);

#if MEMBLOCK_USE_MUTEXES
    pthread_mutex_t *pMutex = (pthread_mutex_t *)0;

    if (opts & MEMBLOCK_OPTION_USE_GLOBAL_MUTEX)
    {
        pthread_mutex_lock(&allocLock);
    }

    if (opts & MEMBLOCK_OPTION_HAS_PTR)
    {
        pDataBlockStart = (char *)pDataBlockStart + MEMBLOCK_INTERNAL_VOIDPTR_SIZE;
    }

    if (opts & MEMBLOCK_OPTION_USE_MUTEX)
    {
        pMutex = (pthread_mutex_t *)pDataBlockStart;
        pDataBlockStart = (char *)pDataBlockStart + sizeof(pthread_mutex_t);

        pthread_mutex_lock(pMutex);
    }
#endif
    // Need to read again to have actual info about used items
    opts = *(uint64_t *)pBlockStart;

    size_t used = opts & ((uint64_t)0xFFFFFFFF);
    const size_t all = used >> 16;
    used &= 0xFFFF;

    void *pRes = MEMBLOCK_NULLPTR;

    used++;

    if (used <= all)
    {
        uint64_t *pMemBits = (uint64_t *)pDataBlockStart;

        // Search bit field
        size_t bitfieldIdx = 0;
        const size_t numBlocks = (all + 63) >> 6;

#define MEMBLOCK_INTERNAL_ALLBITS_USED (~((uint64_t)0))

        // Skip fully used slots
        for (; bitfieldIdx < numBlocks && pMemBits[bitfieldIdx] == MEMBLOCK_INTERNAL_ALLBITS_USED; bitfieldIdx++)
        {
        }
#undef MEMBLOCK_INTERNAL_ALLBITS_USED

        if (bitfieldIdx < numBlocks)
        {
            size_t itemIdx = bitfieldIdx << 6;
            uint64_t bits = pMemBits[bitfieldIdx];

            while (bits & 0x1)
            {
                itemIdx++;
                bits >>= 1;
            }

            if (itemIdx < (bitfieldIdx + 1) << 6)
            {
                const size_t itemSize = (opts >> MEMBLOCK_INTERNAL_NUM_MEMBLOCKS_SHIFT) & 0xFFFF;
                size_t realItemSize = itemSize;

                MEMBLOCK_CALC_REAL_ITEM_SIZE(realItemSize, opts)

                opts &= ~((uint64_t)0xFFFF);
                opts |= used;

                *(uint64_t *)pBlockStart = opts;

                bits = ((uint64_t)0x1) << (itemIdx & 0x3F);
                pMemBits[bitfieldIdx] |= bits;

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
                        pResChar += i;
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

#if MEMBLOCK_USE_MUTEXES
    if (pMutex)
    {
        pthread_mutex_unlock(pMutex);
    }

    if (opts & MEMBLOCK_OPTION_USE_GLOBAL_MUTEX)
    {
        pthread_mutex_unlock(&allocLock);
    }
#endif
    return pRes;
}

/**
 * @param pBlock pointer to managed memory block
 * @returns stored pointer
 */
void *getStoredPointer(void *pBlock)
{
    if (pBlock != MEMBLOCK_NULLPTR)
    {
        void **pPtr = (void **)pBlock;
        void *const pBlockStart = *pPtr;

        if ((char *)pBlockStart >= (char *)pBlock + sizeof(void *))
        {
            uint64_t opts = *(uint64_t *)pBlockStart;
            opts >>= MEMBLOCK_INTERNAL_OPTIONS_SHIFT;

            if (opts & MEMBLOCK_OPTION_HAS_PTR)
            {
                return (void *)((char *)pBlockStart + sizeof(uint64_t));
            }
        }
    }

    return MEMBLOCK_NULLPTR;
}

/**
 * @param pBlock pointer to managed memory block
 * @param ptrToStore pointer to store
 * @returns 0 on success, 1 on error
 */
int setStoredPointer(void *pBlock, void *ptrToStore)
{
    if (pBlock != MEMBLOCK_NULLPTR)
    {
        void **pPtr = (void **)pBlock;
        char *const pBlockStart = (char *)*pPtr;

        if ((char *)pBlockStart >= (char *)pBlock + sizeof(void *))
        {
            uint64_t opts = *(uint64_t *)pBlockStart;
            opts >>= MEMBLOCK_INTERNAL_OPTIONS_SHIFT;

            if (opts & MEMBLOCK_OPTION_HAS_PTR)
            {
                pPtr = (void **)(pBlockStart + sizeof(uint64_t));
                *pPtr = ptrToStore;

                return 0;
            }
        }
    }

    return 1;
}

/**
 * @param pBlock
 * @param pItem
 * @returns 0 on success, 1 on error
 */
int freeItem(void *pBlock, void *pItem)
{
    if ((pBlock != MEMBLOCK_NULLPTR) && (pItem != MEMBLOCK_NULLPTR))
    {
        void **pPtr = (void **)pBlock;
        char *const pBlockStart = (char *)*pPtr;
        uint64_t opts = *(uint64_t *)pBlockStart;

        size_t offset = sizeof(uint64_t);

#if MEMBLOCK_USE_MUTEXES
        if (opts & MEMBLOCK_OPTION_USE_GLOBAL_MUTEX)
        {
            pthread_mutex_lock(&allocLock);
        }

        if (opts & MEMBLOCK_OPTION_HAS_PTR)
        {
            offset += sizeof(void *);
        }

        pthread_mutex_t *pMutex = MEMBLOCK_NULLPTR;

        if (opts & MEMBLOCK_OPTION_USE_MUTEX)
        {
            pMutex = (pthread_mutex_t *)((char *)pBlockStart + offset);
            offset += sizeof(pthread_mutex_t);
            pthread_mutex_lock(pMutex);
        }
#endif
        opts = *(uint64_t *)pBlockStart;
        size_t used = opts & ((uint64_t)0xFFFFFFFF);
        const size_t all = used >> 16;
        used &= 0xFFFF;
        size_t bitfieldSize = ((all + 63) >> 6);
        bitfieldSize *= sizeof(uint64_t);
        uint64_t *const pBitfieldStart = (uint64_t *)((char *)pBlockStart + offset);
        char *const pDataStart = (char *)pBitfieldStart + bitfieldSize;

        int result = 1;
        size_t realItemSize = (opts >> MEMBLOCK_INTERNAL_NUM_MEMBLOCKS_SHIFT) & 0xFFFF;

        if (realItemSize > 0 && used > 0 && ((char *)pItem) >= pDataStart)
        {
            MEMBLOCK_CALC_REAL_ITEM_SIZE(realItemSize, opts)

            const size_t diff = (char *)pItem - pDataStart;
            size_t itemIdx = diff / realItemSize;

            if (itemIdx < all && diff == itemIdx * realItemSize)
            {
                const size_t blockIdx = itemIdx >> 6;
                itemIdx &= 0x3F;
                const uint64_t bit = ((uint64_t)0x1) << itemIdx;

                if (pBitfieldStart[blockIdx] & bit)
                {
                    pBitfieldStart[blockIdx] &= ~bit;
                    --used;

                    opts &= ~((uint64_t)0xFFFF);
                    opts |= used;
                    *(uint64_t *)pBlockStart = opts;
                    result = 0;
                }
            }
        }

#if MEMBLOCK_USE_MUTEXES
        if (pMutex)
        {
            pthread_mutex_unlock(pMutex);
        }

        if (opts & MEMBLOCK_OPTION_USE_GLOBAL_MUTEX)
        {
            pthread_mutex_unlock(&allocLock);
        }
#endif
        return result;
    }
    return 1;
}

/**
 * @param pBlock
 */
void freeBlock(void *pBlock)
{
    if (pBlock == MEMBLOCK_NULLPTR)
    {
        return;
    }

#if MEMBLOCK_USE_MUTEXES
    pthread_mutex_lock(&allocLock);
#endif

#if MEMBLOCK_USE_MUTEXES
    void **pPtr = (void **)pBlock;
    void *pBlockStart = *pPtr;
    const uint64_t opts = *(uint64_t *)pBlockStart;

    if (opts & MEMBLOCK_OPTION_USE_MUTEX)
    {
        size_t mutexOffset = sizeof(uint64_t);

        if (opts & MEMBLOCK_OPTION_HAS_PTR)
        {
            mutexOffset += sizeof(void *);
        }
        pthread_mutex_t *pMutex = (pthread_mutex_t *)((char *)pBlockStart + mutexOffset);
        pthread_mutex_destroy(pMutex);
    }
#endif
    MEMBLOCK_free(pBlock);

#if MEMBLOCK_USE_MUTEXES
    pthread_mutex_unlock(&allocLock);
#endif
}

#endif
#endif