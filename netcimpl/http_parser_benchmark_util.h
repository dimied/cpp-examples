#ifndef HTTP_PARSER_BENCHMARK_UTIL
#define HTTP_PARSER_BENCHMARK_UTIL

double perTestTime(double diff, size_t total_tests)
{
    // we can store upto 2^53 precisely in a double
    unsigned long limit = ((0x1ul << 53) / CLOCKS_PER_SEC) + 10;

    if ((unsigned long)total_tests < limit)
    {
        double div = (double)total_tests;
        div *= (double)CLOCKS_PER_SEC;
        return (diff) / div;
    }
    else
    {
        return diff / (double)total_tests;
    }
}

void printTimes(char *name, clock_t start, clock_t end, size_t total_tests)
{
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    if (total_tests > 0)
    {
        double perTest = perTestTime(((double)(end - start)), total_tests);
        printf("%s: %f|%0.10f\n", name, cpu_time_used, perTest);
    }
    else
    {
        printf("%s: %f\n", name, cpu_time_used);
    }
}

typedef struct TestMemAllocations
{
    int used;
    /*@shared@*/ /*@null@*/ const char *name;
    /*@shared@*/ void *allocations[6];
} TestMemAllocations;

// static TestMemAllocations *testAllocChain = 0;

static char buffer_for_TestAlignedText[(0x1 << 16)];

/*@null@*/ TestMemAllocations *testMemAllocs(char *name)
{
    TestMemAllocations *pM = (TestMemAllocations *)calloc(1, sizeof(TestMemAllocations));
    if (pM != 0)
    {
        pM->name = name;
    }
    return pM;
}

// 16 bits: size, 16 bits: used, 8 bits: offset
typedef uint64_t TestAlignedText_state;

typedef struct TestAlignedText
{
    TestAlignedText_state state;
    char data[];
} TestAlignedText;

#define TEST_ALIGNED_TEXT_OK 0
#define TEST_ALIGNED_TEXT_ERROR_TOO_LARGE 1
#define TEST_ALIGNED_TEXT_ERROR_NOMEM 2
#define TEST_ALIGNED_TEXT_ERROR_INVALID_ARGS 3

int freeTestAlignedText(/*@null@*/ TestAlignedText *pAT)
{
    if (pAT != 0)
    {
        free(pAT);
        return TEST_ALIGNED_TEXT_OK;
    }
    return TEST_ALIGNED_TEXT_ERROR_INVALID_ARGS;
}

/*@null@*/ TestAlignedText *allocTestAlignedText(size_t sz, /*@null@*/ int *pError)
{
    int error = TEST_ALIGNED_TEXT_OK;

    TestAlignedText *result = 0;
    if (sz > 0xFFFF)
    {
        error = TEST_ALIGNED_TEXT_ERROR_TOO_LARGE;
    }
    else
    {
        size_t real_size = (sz + 7);
        real_size -= (real_size & 0x7);
        size_t alloc_size = real_size + sizeof(TestAlignedText_state);
        void *mem = calloc(alloc_size >> 2, 4);
        if (mem != 0)
        {
            result = (TestAlignedText *)mem;
            result->state = real_size;
        }
        else
        {
            error = TEST_ALIGNED_TEXT_ERROR_NOMEM;
        }
    }
    if (pError != 0)
    {
        *pError = error;
    }

    return result;
}

char *get_TestAlignedText(TestAlignedText *pAT)
{
    if (pAT == 0)
    {
        return 0;
    }
    char *pDst = (char *)(pAT) + sizeof(TestAlignedText_state);
    const size_t cur_offset = pAT->state >> 32;
    return (pDst + cur_offset);
}

int realign_TestAlignedText(TestAlignedText *pAT, size_t offset)
{
    if (pAT == 0 || offset > 255)
    {
        return TEST_ALIGNED_TEXT_ERROR_INVALID_ARGS;
    }
    size_t cur_offset = (pAT->state >> 32) & 0xFF;

    if (cur_offset == offset)
    {
        return TEST_ALIGNED_TEXT_OK;
    }

    size_t used = pAT->state;
    const size_t cur_size = used & 0xFFFF;
    used = (used >> 16) & 0xFFFF;

    if ((offset + used) > cur_size)
    {
        return TEST_ALIGNED_TEXT_ERROR_NOMEM;
    }
    char *pSrc = get_TestAlignedText(pAT);
    char *pDst = (char *)(pAT) + sizeof(TestAlignedText_state);

    memcpy(buffer_for_TestAlignedText, pSrc, used);
    memset(pDst, 0, cur_size);

    pDst += offset;
    memcpy(pDst, buffer_for_TestAlignedText, used);

    used = pAT->state & 0xFFFFFFFFul;
    pAT->state = used | (offset << 32);

    return TEST_ALIGNED_TEXT_OK;
}

int setTestAlignedText(TestAlignedText *pAT, char *text, size_t len, size_t padding)
{
    if (pAT == 0 || text == 0)
    {
        return TEST_ALIGNED_TEXT_ERROR_INVALID_ARGS;
    }
    const size_t cur_size = (pAT->state & 0xFFFF);
    if ((len + padding) > cur_size)
    {
        return TEST_ALIGNED_TEXT_ERROR_TOO_LARGE;
    }
    // NO offset!
    char *pDst = (char *)(pAT) + sizeof(TestAlignedText_state);
    memset(pDst, 0, cur_size);
    memcpy(pDst, text, len);

    len += padding;
    pAT->state &= 0xFFFF;
    pAT->state |= (len << 16);

    return TEST_ALIGNED_TEXT_OK;
}

static /*@null@*/ void *allocTestMem(TestMemAllocations *pM, size_t nmemb, size_t sizePerItem)
{
    if (pM != 0)
    {
        printf("ALLOC:%s|#items=%d|#size=%d\n", pM->name, (int)nmemb, (int)sizePerItem);

        if (pM->used == (int)(sizeof(pM->allocations) / sizeof(pM->allocations[0])))
        {
            printf("allocTestMem: Too many\n");
            return 0;
        }

        void *mem = calloc(nmemb, sizePerItem);
        if (mem != 0)
        {
            pM->allocations[pM->used] = mem;
            pM->used++;
        }
        return mem;
    }
    return 0;
}

static void freeMemAllocs(/*@null@*/ TestMemAllocations *pM)
{
    if (pM != 0)
    {
        printf("FREE-MEM: '%s'\n", pM->name ? pM->name : "");
        int i;
        for (i = 0; i < pM->used; i++)
        {
            if (pM->allocations[i] != (void *)0)
            {
                free(pM->allocations[i]);
                pM->allocations[i] = (void *)0;
            }
        }
        /*pM->name = 0;
        pM->used = 0;
        */
        free(pM);
    }
}
void freeMemAllocsWithMessage(TestMemAllocations *pM, const char *msg)
{
    if (msg != 0)
    {
        printf("%s", msg);
    }
    freeMemAllocs(pM);
}

void realign_values(char **tests, char **testsReal, size_t offset, size_t sz)
{

    size_t alignIdx, hits = 0;
    char buffer[128];

    for (alignIdx = 0; alignIdx < sz; alignIdx++)
    {
        char *pSrc = tests[alignIdx];
        char *p = pSrc;
        size_t di = 0;
        while (*p == '\0')
        {
            ++di;
            ++p;
        }
        // already aligned?
        if (di == offset)
        {
            ++hits;
            continue;
        }

        memset(buffer, 0, 128);
        di = 0;
        while (*p != '\0')
        {
            buffer[di] = *p;
            ++di;
            ++p;
        }
        memset(pSrc, 0, 32);
        p = pSrc + offset;
        memcpy(p, buffer, di);
        testsReal[alignIdx] = p;
    }
}
#endif