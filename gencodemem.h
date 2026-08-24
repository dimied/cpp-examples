#ifndef GEN_CODE_MEMORY
#define GEN_CODE_MEMORY

#include <string.h>

#define MAX_NUM_ALLOCATIONS (100 * 1024)
void *genCodeMemoryPointers[MAX_NUM_ALLOCATIONS];
unsigned char genCodeTypes[MAX_NUM_ALLOCATIONS];
unsigned char genCodeSources[MAX_NUM_ALLOCATIONS];
unsigned char genCodeOptions[MAX_NUM_ALLOCATIONS];
size_t genCodeSizes[MAX_NUM_ALLOCATIONS];
char *genCodeTypeNames[256];
char *genCodeSourceNames[256];

int initMemCalled = 0;
size_t numMemAllocItems = 0;

char *unknownMemType = "?";

int printAllocationStats(char *pDst, size_t dstLen)
{
    const size_t numAllocs = numMemAllocItems;
    size_t totalDel = 0;
    size_t maxType = 0;

    size_t typeCounts[256][2];
    memset(typeCounts, 0, sizeof(typeCounts));

    for (size_t i = 0; i < numAllocs; i++)
    {
        size_t t = genCodeTypes[i] & 0x7F; // ignore clear flag

        if (maxType < t)
        {
            maxType = t;
        }
        typeCounts[t][0]++;

        if ((genCodeOptions[i] & 0x1))
        {
            totalDel++;
            typeCounts[t][1]++;
        }
    }

    if (pDst != 0)
    {
        size_t s = snprintf(pDst, dstLen, "Mem: total=%ld | #del=%ld\n", numAllocs, totalDel);
        if (s < dstLen)
        {
            return 0;
        }
        return 1;
    }
    printf("Mem: total=%ld | #del=%ld\n", numAllocs, totalDel);
    printf("Mem: max type = %ld\n", maxType);

    for (size_t i = 0; i <= maxType; i++)
    {
        char *pType = genCodeTypeNames[i];
        if (pType == 0)
        {
            pType = unknownMemType;
        }
        printf("T: %10s: alloc: %ld | del: %ld\n", pType, typeCounts[i][0], typeCounts[i][1]);
    }

    return 0;
}

void memSetTypeName(unsigned char type, char *name)
{
    genCodeTypeNames[type] = name;
}

void memSetSourceName(unsigned char type, char *name)
{
    genCodeSourceNames[type] = name;
}

void initMem()
{
    if (initMemCalled == 0)
    {
        memset(genCodeMemoryPointers, 0, sizeof(genCodeMemoryPointers));
        memset(genCodeTypes, 0, sizeof(genCodeTypes));
        memset(genCodeSizes, 0, sizeof(genCodeSizes));
        memset(genCodeSources, 0, sizeof(genCodeSources));
        memset(genCodeOptions, 0, sizeof(genCodeOptions));
        memset(genCodeTypeNames, 0, sizeof(genCodeTypeNames));
        memset(genCodeSourceNames, 0, sizeof(genCodeSourceNames));
    }
    ++initMemCalled;
}

void *allocMemoryImpl(size_t size, unsigned char type, unsigned char source, int *pItemNum)
{
    if (numMemAllocItems < MAX_NUM_ALLOCATIONS)
    {
        void *result = malloc(size);
        if (result != 0)
        {
            genCodeMemoryPointers[numMemAllocItems] = result;
            genCodeSizes[numMemAllocItems] = size;
            genCodeTypes[numMemAllocItems] = type;
            genCodeSources[numMemAllocItems] = source;

            if (pItemNum != 0)
            {
                *pItemNum = numMemAllocItems;
            }
            ++numMemAllocItems;
        }
        return result;
    }
    return 0;
}

void *allocMemory(size_t size, unsigned char type, unsigned char source)
{
    if (type > 0x7F)
    {
        printf("Invalid type alloc!\n");
    }
    return allocMemoryImpl(size, type, source, NULL);
}

void *callocMemory(size_t size, unsigned char type, unsigned char source)
{
    // mark type as zero initialized
    void *result = allocMemoryImpl(size, type | 0x80, source, NULL);
    if (result != 0)
    {
        memset(result, 0, size);
    }
    return result;
}

void freeMemory(void *pMem)
{
    if (pMem != 0)
    {
        for (size_t i = 0; i < numMemAllocItems; i++)
        {
            if (pMem == genCodeMemoryPointers[i])
            {
                if ((genCodeOptions[i] & 0x1) == 0)
                {
                    free(pMem);
                    genCodeOptions[i] |= 1;
                    return;
                    // genCodeMemoryPointers[i] = (void *)0;
                }
                else
                {
                    printf("Trying to deallocate twice @%ld | %p\n", i, pMem);
                }

                return;
            }
        }
        printf("Failed to find memory: %p | %s\n", pMem, (char *)pMem);
    }
}

/**
 * @param len
 */
void *mallocZeroInit(size_t len)
{
    return calloc(len, 1);
}

#endif