#ifndef BENCH_HEADERS_SNIPPETS
#define BENCH_HEADERS_SNIPPETS

#include <string.h>

int impl_headers_normal(char *pathStart)
{

    size_t idx = 0;
    for (idx = 0; idx < NUM_HEADER_HANDLERS; idx++)
    {
        if (0 == strcmp(pathStart, headerHandlers[idx].name))
        {
            return idx;
        }
    }
    return idx;
}

size_t lastSize = 0;

int impl_headers_aligned(char *pathStart)
{
    // uint64_t v = (uint64_t)pathStart;

    const size_t s = strlen(pathStart);

    if (s < HEADER_NAME_MIN_LEN || s > HEADER_NAME_MAX_LEN)
    {
        lastSize = s;
        return NUM_HEADER_HANDLERS;
    }
    {
        const uint64_t bit = (0x1ul) << (s - 1);
        if ((bit & HEADER_SIZE_BITS_CONSTANT) == 0)
        {
            return NUM_HEADER_HANDLERS;
        }
    }

    uint64_t offsets = headerNamesOffsets8[s];
    uint64_t values = headerNamesValues8[s];

    while (offsets != 0)
    {
        uint64_t off = (offsets & 0xFF) << 3; //8 byte alignment
        const char *pS = &headerNamesAligned8[off];
        
        if (0 == strncmp(pathStart, pS, s))
        {
            return (values & 0xFF);
        }

        offsets >>= 8;
        values >>= 8;
    }

    return NUM_HEADER_HANDLERS;
}

void bench_parse_headers()
{
    printf("----\n");
    double cpu_time_used;
    void *mem_allocations[5] = {0, 0, 0, 0, 0};

    clock_t end, start = clock();

    int *results = (int *)calloc(NUM_BENCH_HEADERS_ENTRIES, sizeof(int));
    mem_allocations[0] = (void *)results;
    if (results == 0)
    {
        printf("MEM-ALLOC-FAILED: results\n");
        return;
    }

    int *expected = (int *)calloc(NUM_BENCH_HEADERS_ENTRIES, sizeof(int));
    if (expected == 0)
    {
        printf("MEM-ALLOC-FAILED: expected\n");
        BENCH_FREE_MEM(mem_allocations);
        return;
    }
    mem_allocations[1] = (void *)expected;

    char **tests = (char **)calloc(NUM_BENCH_HEADERS_ENTRIES, sizeof(char *));
    if (tests == 0)
    {
        printf("MEM-ALLOC-FAILED: tests\n");
        BENCH_FREE_MEM(mem_allocations);
        return;
    }
    mem_allocations[2] = (void *)tests;

    char **testsReal = (char **)calloc(NUM_BENCH_HEADERS_ENTRIES, sizeof(char *));
    if (testsReal == 0)
    {
        printf("MEM-ALLOC-FAILED: tests\n");
        BENCH_FREE_MEM(mem_allocations);
        return;
    }
    mem_allocations[3] = (void *)testsReal;

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("MEMORY! %f\n", cpu_time_used);

    start = clock();
    size_t i;
    for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
    {
        char *p = (char *)calloc(1, 48);
        if (p == 0)
        {
            break;
        }
        // different strings
        int r = rand();
        r %= NUM_HEADER_HANDLERS;
        char *pM = headerHandlers[r].name;
        size_t l = strlen(pM);
        memcpy(p, pM, l);

        if (i != 0 && i % 34 == 0)
        {
            int r2 = rand();
            p[r2 % l] = 'X';
            expected[i] = NUM_HEADER_HANDLERS;
        }
        else
        {
            expected[i] = r;
        }
#if 0
                if (i < 10)
                {
                    printf("M:%ld|%s|\n", i, p);
                }
#endif
        tests[i] = p;
    }

    memcpy(testsReal, tests, NUM_BENCH_HEADERS_ENTRIES * sizeof(char *));

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("TESTS! %f\n", cpu_time_used);

    if (i != NUM_BENCH_HEADERS_ENTRIES)
    {
        while (i > 0)
        {
            if (tests[i] != 0)
            {
                free(tests[i]);
            }
        }
        printf("MEM-ALLOC-FAILED: tests generation\n");
        BENCH_FREE_MEM(mem_allocations);
        return;
    }

    size_t total = NUM_BENCH_HEADERS_ENTRIES * NUM_BENCH_HEADERS_RUNS;

    printf("CASES!#runs:%d|#tests:%d|#total:%ld\n", NUM_BENCH_HEADERS_RUNS, NUM_BENCH_HEADERS_ENTRIES, total);

#define NUM_BENCH_HEADERS_IMPLEMENTATIONS 2

    double times[8][NUM_BENCH_HEADERS_IMPLEMENTATIONS];
    memset(times, 0, sizeof(double) * NUM_BENCH_HEADERS_IMPLEMENTATIONS * 8);

    size_t offset;

#define BENCH_PROTOCOL_OFFSET_LIMIT 8

    for (offset = 0; offset < BENCH_PROTOCOL_OFFSET_LIMIT; offset++)
    {
        // align values
        start = clock();
        size_t alignIdx, hits = 0;
        char buffer[32];

        for (alignIdx = 0; alignIdx < NUM_BENCH_HEADERS_ENTRIES; alignIdx++)
        {
            char *pSrc = tests[alignIdx];
            char *p = pSrc;
            size_t di = 0;
            while (*p == 0)
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

            memset(buffer, 0, 32);
            di = 0;
            while (*p != 0)
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

        end = clock();

        printf("Align:%ld", offset);
        printTimes("", start, end, total);
        printf("Hits: %ld\n", hits);
        //
        //

        size_t testRun;
        printf("Normal...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_RUNS; testRun++)
        {
            for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
            {
                char *p = testsReal[i];
                int r = impl_headers_normal(p);
                if (r != expected[i])
                {
                    printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, testsReal[i], expected[i], r);
                    break;
                }
            }
        }

        end = clock();
        times[offset][0] = ((double)(end - start));
        // printTimes("Normal", start, end, total);
#if 1
        printf("IMP1...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_RUNS; testRun++)
        {
            for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
            {
                int r = impl_headers_aligned(testsReal[i]);
                if (r != expected[i])
                {
                    printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, testsReal[i], expected[i], r);
                    printf("LAST-SIZE:%ld\n", lastSize);
                    printf("T:%s|%s|\n", tests[i], testsReal[i]);
                    break;
                }
            }
        }

        end = clock();
        // printTimes("IMP1", start, end, total);
        times[offset][1] = ((double)(end - start));
#endif
    }

    printf("Offset\t");

    char *names[NUM_BENCH_HEADERS_IMPLEMENTATIONS] = {"Norm", "IMP1"};

    for (i = 0; i < NUM_BENCH_HEADERS_IMPLEMENTATIONS; i++)
    {
        printf("\t%12s", names[i]);
    }
    printf("\n");

    for (offset = 0; offset < BENCH_PROTOCOL_OFFSET_LIMIT; offset++)
    {
        printf("%6ld\t", offset);
        for (i = 0; i < NUM_BENCH_HEADERS_IMPLEMENTATIONS; i++)
        {
            double t = times[offset][i] / CLOCKS_PER_SEC;
            printf("\t%0.10f", t);
        }
        printf("\n");
    }
    printf("----\n");
    for (offset = 0; offset < BENCH_PROTOCOL_OFFSET_LIMIT; offset++)
    {
        printf("%5ld+\t", offset);
        for (i = 0; i < NUM_BENCH_HEADERS_IMPLEMENTATIONS; i++)
        {
            double t = perTestTime(times[offset][i], total);
            printf("\t%0.10f", t);
        }
        printf("\n");
    }
    printf("\n");

#undef BENCH_PROTOCOL_OFFSET_LIMIT

    printf("FREE TESTS!\n");
    for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
    {
        if (tests[i] != 0)
        {
            free(tests[i]);
        }
    }
    BENCH_FREE_MEM(mem_allocations);

    printf("----\n");
#undef NUM_BENCH_HEADERS_IMPLEMENTATIONS
}
#endif