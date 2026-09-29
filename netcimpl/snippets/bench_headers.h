#ifndef BENCH_HEADERS_SNIPPETS
#define BENCH_HEADERS_SNIPPETS

#include <string.h>

static size_t lastSize = 0;

#define bench_header_names_lc(idx) (header_comp_data.header_names_lowercase[idx])

#define bench_header_names header_comp_data.header_names_aligned8
#define bench_header_offsets header_comp_data.header_names_offsets8
#define bench_header_values header_comp_data.header_names_values8
#define bench_header_signatures header_comp_data.header_names_signatures8

#if HEADER_DATA_SORTED_NAMES
#define bench_header_names_sorted header_comp_data.header_names_aligned8_sorted
#define bench_header_offsets_sorted header_comp_data.header_names_offsets8_sorted
#endif

static int impl_headers_normal(char *pathStart)
{
    int idx = 0;

    for (idx = 0; idx < (int)NUM_HEADER_HANDLERS; idx++)
    {
        if (0 == strcmp(pathStart, header_comp_data.header_names_lowercase[idx]))
        {
            return idx + 1;
        }
    }
    return idx + 1;
}

static int impl_headers_aligned(char *pathStart)
{
    const size_t s = strlen(pathStart);

    if (s < HEADER_NAME_MIN_LEN || s > HEADER_NAME_MAX_LEN)
    {
        lastSize = s;
        return (int)NUM_HEADER_HANDLERS + 1;
    }
    {
        const uint64_t bit = (0x1ul) << (s - 1);
        if ((bit & HEADER_SIZE_BITS_CONSTANT) == 0)
        {
            return (int)NUM_HEADER_HANDLERS + 1;
        }
    }

    uint64_t offsets = bench_header_offsets[s];
    uint64_t values = bench_header_values[s];

    while (offsets != 0)
    {
        uint64_t off = (offsets & 0xFF) << 3; // 8 byte alignment
        const char *pS = &bench_header_names[off];

        if (0 == strncmp(pathStart, pS, s))
        {
            return (int)(values & 0xFF);
        }

        offsets >>= 8;
        values >>= 8;
    }

    return (int)NUM_HEADER_HANDLERS + 1;
}

static int impl_headers_aligned2(char *pathStart)
{
    const size_t s = strlen(pathStart);

    if (s < HEADER_NAME_MIN_LEN || s > HEADER_NAME_MAX_LEN)
    {
        lastSize = s;
        return (int)NUM_HEADER_HANDLERS + 1;
    }
    // size test
    {
        const uint64_t bit = (0x1ul) << (s - 1);
        if ((bit & HEADER_SIZE_BITS_CONSTANT) == 0)
        {
            return (int)NUM_HEADER_HANDLERS + 1;
        }
    }

    uint64_t offsets = bench_header_offsets[s];
    uint64_t values = bench_header_values[s];

    if (s > 8)
    {
        const int s2 = (s + 7) >> 3;

        uint64_t *vals = (uint64_t *)&bench_header_names[0];
        uint64_t *pVals = ((uint64_t *)pathStart);

        while (offsets != 0)
        {
            uint64_t *checkValues = vals + (offsets & 0xFF);
            int ss = (int)s2;

            uint64_t *pVals2 = pVals;

            while (ss != 0)
            {
                if (*pVals2 != *checkValues)
                {
                    break;
                }
                ++pVals2;
                ++checkValues;
                --ss;
            }
            if (ss == 0)
            {
                return (int)(values & 0xFF);
            }

            offsets >>= 8;
            values >>= 8;
        }
    }
    else
    {
        const uint64_t val = *((uint64_t *)pathStart);
        uint64_t *vals = (uint64_t *)&bench_header_names[0];

        while (offsets != 0)
        {
            uint64_t checkValue = vals[(offsets & 0xFF)];
            if (checkValue == val)
            {
                return (int)(values & 0xFF);
            }

            offsets >>= 8;
            values >>= 8;
        }
    }

    return NUM_HEADER_HANDLERS + 1;
}

#if HEADER_NAME_MAX_LEN > 40
#error "Unrolled version does't support strings of length greater than 40"
#else

static int impl_headers_aligned3(char *pathStart)
{
    const size_t s = strlen(pathStart);

    if (s < HEADER_NAME_MIN_LEN || s > HEADER_NAME_MAX_LEN)
    {
        // lastSize = s;
        return NUM_HEADER_HANDLERS + 1;
    }
    // size test
    {
        const uint64_t bit = (0x1ul) << (s - 1);
        if ((bit & HEADER_SIZE_BITS_CONSTANT) == 0)
        {
            return NUM_HEADER_HANDLERS + 1;
        }
    }

    uint64_t offsets = bench_header_offsets[s];
    uint64_t values = bench_header_values[s];

    if (s > 8)
    {
        const int s2 = (s + 7) >> 3;

        uint64_t *vals = (uint64_t *)&bench_header_names[0];
        uint64_t *pVals = ((uint64_t *)pathStart);
        int sawPrefix = 0;

        // using loop unrolling
        while (offsets != 0)
        {
            uint64_t *checkValues = vals + (offsets & 0xFF);
            // uint64_t myValue = *pVals, checkValue = *checkValues;

            // 8 bytes equal?
            // if (myValue == checkValue)
            if (*pVals == *checkValues)
            {
                sawPrefix |= 1;

                uint64_t *pVals2 = pVals + 1;
                ++checkValues;
                // 16 bytes equal?
                if (*pVals2 == *checkValues)
                {
                    sawPrefix |= 2;

                    if (s2 == 2)
                    {
                        return (int)(values & 0xFF);
                    }
#if HEADER_NAME_MAX_LEN > 16
                    else
                    {
                        ++pVals2;
                        ++checkValues;
                        // 24 bytes equal?
                        if (*pVals2 == *checkValues)
                        {
                            sawPrefix |= 4;
                            if (s2 == 3)
                            {
                                return (int)(values & 0xFF);
                            }
#if HEADER_NAME_MAX_LEN > 24
                            else
                            {
                                ++pVals2;
                                ++checkValues;
                                // 32 bytes equal?
                                if (*pVals2 == *checkValues)
                                {
                                    if (s2 == 4)
                                    {
                                        return (int)(values & 0xFF);
                                    }
#if HEADER_NAME_MAX_LEN > 32
                                    else
                                    {
                                        ++pVals2;
                                        ++checkValues;
                                        // 33-40 bytes equal?
                                        if (*pVals2 == *checkValues)
                                        {
                                            return (int)(values & 0xFF);
                                        }
                                    }
#endif
                                }
                            }
#endif
                        }
                        else if ((sawPrefix & 0x4) != 0)
                        {
                            break;
                        }
                    }
#endif
                }
                else if ((sawPrefix & 0x2) != 0)
                {
                    break;
                }
            }
            else if (sawPrefix != 0)
            {
                break;
            }

            offsets >>= 8;
            values >>= 8;
        }
    }
    else
    {
        const uint64_t val = *((uint64_t *)pathStart);
        uint64_t *vals = (uint64_t *)&bench_header_names[0];

        while (offsets != 0)
        {
            uint64_t checkValue = vals[(offsets & 0xFF)];
            if (checkValue == val)
            {
                return (values & 0xFF);
            }

            offsets >>= 8;
            values >>= 8;
        }
    }

    return NUM_HEADER_HANDLERS + 1;
}

//static size_t sigHits = 0;

static int impl_headers_aligned4(const char *pathStart, uint64_t sig)
{
    size_t s = strlen(pathStart);

    if (s < HEADER_NAME_MIN_LEN || s > HEADER_NAME_MAX_LEN)
    {
        lastSize = s;
        return NUM_HEADER_HANDLERS + 1;
    }
    // size test
    {
        const uint64_t bit = (0x1ul) << (s - 1);
        if ((bit & HEADER_SIZE_BITS_CONSTANT) == 0)
        {
            return NUM_HEADER_HANDLERS + 1;
        }
    }

    // some invalid chars?
    if ((bench_header_signatures[s] & sig) != sig)
    {
        // printf("SIG:%s|%ld <> %ld\n", pathStart, sig, header_names_signatures8[s]);
        return NUM_HEADER_HANDLERS + 1;
    }

    uint64_t offsets = bench_header_offsets[s];
    uint64_t values = bench_header_values[s];

    if (s > 8)
    {
        s = (s + 7) >> 3;

#define s2 (s)

        uint64_t *vals = (uint64_t *)&bench_header_names[0];
        uint64_t *pVals = ((uint64_t *)pathStart);
        int sawPrefix = 0;
        const uint64_t myValue = *pVals;

        // using loop unrolling
        while (offsets != 0)
        {
            uint64_t *checkValues = vals + (offsets & 0xFF);

            // 8 bytes equal?
            // if (myValue == checkValue)
            if (myValue == *checkValues)
            {
                sawPrefix |= 1;

                uint64_t *pVals2 = pVals + 1;
                ++checkValues;
                // 16 bytes equal?
                if (*pVals2 == *checkValues)
                {
                    sawPrefix |= 2;

                    if (s2 == 2)
                    {
                        return (int)(values & 0xFF);
                    }
#if HEADER_NAME_MAX_LEN > 16
                    else
                    {
                        ++pVals2;
                        ++checkValues;
                        // 24 bytes equal?
                        if (*pVals2 == *checkValues)
                        {
                            sawPrefix |= 4;
                            if (s2 == 3)
                            {
                                return (int)(values & 0xFF);
                            }
#if HEADER_NAME_MAX_LEN > 24
                            else
                            {
                                ++pVals2;
                                ++checkValues;
                                // 32 bytes equal?
                                if (*pVals2 == *checkValues)
                                {
                                    if (s2 == 4)
                                    {
                                        return (int)(values & 0xFF);
                                    }
#if HEADER_NAME_MAX_LEN > 32
                                    else
                                    {
                                        ++pVals2;
                                        ++checkValues;
                                        // 33-40 bytes equal?
                                        if (*pVals2 == *checkValues)
                                        {
                                            return (int)(values & 0xFF);
                                        }
                                    }
#endif
                                }
                            }
#endif
                        }
                        else if ((sawPrefix & 0x4) != 0)
                        {
                            break;
                        }
                    }
#endif
                }
                else if ((sawPrefix & 0x2) != 0)
                {
                    break;
                }
            }
            else if (sawPrefix != 0)
            {
                break;
            }

            offsets >>= 8;
            values >>= 8;
        }
#undef s2
    }
    else
    {
        const uint64_t val = *((uint64_t *)pathStart);
        uint64_t *vals = (uint64_t *)&bench_header_names[0];

        while (offsets != 0)
        {
            uint64_t checkValue = vals[(offsets & 0xFF)];
            if (checkValue == val)
            {
                return (int)(values & 0xFF);
            }

            offsets >>= 8;
            values >>= 8;
        }
    }

    return NUM_HEADER_HANDLERS + 1;
}
#endif

#if HEADER_DATA_SORTED_NAMES
static int impl_headers_sorted_by_size(char *pathStart, uint64_t sig)
{
    size_t s = strlen(pathStart);

    if (s < HEADER_NAME_MIN_LEN || s > HEADER_NAME_MAX_LEN)
    {
        lastSize = s;
        return NUM_HEADER_HANDLERS;
    }
    // size test
    {
        const uint64_t bit = (0x1ul) << (s - 1);
        if ((bit & HEADER_SIZE_BITS_CONSTANT) == 0)
        {
            return NUM_HEADER_HANDLERS;
        }
    }
    // some invalid chars?
    if ((bench_header_signatures[s] & sig) != sig)
    {
        // printf("SIG:%s|%ld <> %ld\n", pathStart, sig, header_names_signatures8[s]);
        return NUM_HEADER_HANDLERS;
    }

    uint64_t offsets = bench_header_offsets_sorted[s];

    if (offsets == 0)
    {
        return NUM_HEADER_HANDLERS;
    }
    // uint64_t values = bench_header_values[s];

    uint64_t *checkValues = (uint64_t *)&bench_header_names_sorted[0];
    checkValues += (offsets & 0xFF);
    offsets >>= 8;

    // printf("C: %s <?> %s\n", pathStart, (char *)vals);

    if (s > 8)
    {
        const int s2 = (s + 7) >> 3;

        uint64_t *pVals = ((uint64_t *)pathStart);

        const uint64_t myValue = *pVals;

        size_t count = offsets & 0xFF;
        offsets >>= 8;

        int sawPrefix = 0;

        while (count != 0)
        {
            uint64_t *next = checkValues + s2;

            if (myValue == *checkValues)
            {
                sawPrefix |= 1;
                ++checkValues;
                uint64_t *pVals2 = pVals + 1;
#if 0
                while (vals != next && *vals == *pVals2)
                {
                    ++vals;
                    ++pVals2;
                }
                if (vals == next)
                {
                    return (offsets & 0xFF);
                }
#else
                // TODO: Loop unrolling
                // 16 bytes equal?
                if (*pVals2 == *checkValues)
                {
                    sawPrefix |= 2;

                    if (s2 == 2)
                    {
                        return (offsets & 0xFF);
                    }
#if HEADER_NAME_MAX_LEN > 16
                    else
                    {
                        ++pVals2;
                        ++checkValues;
                        // 24 bytes equal?
                        if (*pVals2 == *checkValues)
                        {
                            sawPrefix |= 4;
                            if (s2 == 3)
                            {
                                return (offsets & 0xFF);
                            }
#if HEADER_NAME_MAX_LEN > 24
                            else
                            {
                                ++pVals2;
                                ++checkValues;
                                // 32 bytes equal?
                                if (*pVals2 == *checkValues)
                                {
                                    if (s2 == 4)
                                    {
                                        return (offsets & 0xFF);
                                    }
#if HEADER_NAME_MAX_LEN > 32
                                    else
                                    {
                                        ++pVals2;
                                        ++checkValues;
                                        // 33-40 bytes equal?
                                        if (*pVals2 == *checkValues)
                                        {
                                            return (offsets & 0xFF);
                                        }
                                    }
#endif
                                }
                            }
#endif
                        }
                        else if ((sawPrefix & 0x4) != 0)
                        {
                            break;
                        }
                    }
#endif
                }
                else if ((sawPrefix & 0x2) != 0)
                {
                    break;
                }
#endif
            }
            else if (sawPrefix != 0)
            {
                return NUM_HEADER_HANDLERS;
            }
            checkValues = next;
            --count;
            offsets >>= 8;
        }
#if 0
        // using loop unrolling
        while (offsets != 0)
        {
            uint64_t *checkValues = vals + (offsets & 0xFF);

            // 8 bytes equal?
            // if (myValue == checkValue)
            if (myValue == *checkValues)
            {
                sawPrefix |= 1;

                uint64_t *pVals2 = pVals + 1;
                ++checkValues;
                // 16 bytes equal?
                if (*pVals2 == *checkValues)
                {
                    sawPrefix |= 2;

                    if (s2 == 2)
                    {
                        return (values & 0xFF);
                    }
#if HEADER_NAME_MAX_LEN > 16
                    else
                    {
                        ++pVals2;
                        ++checkValues;
                        // 24 bytes equal?
                        if (*pVals2 == *checkValues)
                        {
                            sawPrefix |= 4;
                            if (s2 == 3)
                            {
                                return (values & 0xFF);
                            }
#if HEADER_NAME_MAX_LEN > 24
                            else
                            {
                                ++pVals2;
                                ++checkValues;
                                // 32 bytes equal?
                                if (*pVals2 == *checkValues)
                                {
                                    if (s2 == 4)
                                    {
                                        return (values & 0xFF);
                                    }
#if HEADER_NAME_MAX_LEN > 32
                                    else
                                    {
                                        ++pVals2;
                                        ++checkValues;
                                        // 33-40 bytes equal?
                                        if (*pVals2 == *checkValues)
                                        {
                                            return (values & 0xFF);
                                        }
                                    }
#endif
                                }
                            }
#endif
                        }
                        else if ((sawPrefix & 0x4) != 0)
                        {
                            break;
                        }
                    }
#endif
                }
                else if ((sawPrefix & 0x2) != 0)
                {
                    break;
                }
            }
            else if (sawPrefix != 0)
            {
                break;
            }

            offsets >>= 8;
            values >>= 8;
        }
#endif
    }
    else
    {
        const uint64_t val = *((uint64_t *)pathStart);
        s = offsets & 0xFF;
        offsets >>= 8;

        while (s != 0)
        {
            if (val == *checkValues)
            {
                return (offsets & 0xFF);
            }
            ++checkValues;
            --s;
            offsets >>= 8;
        }
    }

    return NUM_HEADER_HANDLERS;
}

#endif

void bench_parse_headers()
{
    printf("----\n");
    double cpu_time_used;
    TestMemAllocations *pM = testMemAllocs("bench_headers");

    if (pM == 0)
    {
        printf("MEM-ALLOC-FAILED: base\n");
        return;
    }

    clock_t end, start = clock();

    int *results = (int *)allocTestMem(pM, NUM_BENCH_HEADERS_ENTRIES, sizeof(int));
    if (results == 0)
    {
        printf("MEM-ALLOC-FAILED: results\n");
        return;
    }

    int *expected = (int *)allocTestMem(pM, NUM_BENCH_HEADERS_ENTRIES, sizeof(int));
    if (expected == 0)
    {
        freeMemAllocsWithMessage(pM, "MEM-ALLOC-FAILED: expected\n");
        return;
    }

    char **tests = (char **)allocTestMem(pM, NUM_BENCH_HEADERS_ENTRIES, sizeof(char *));
    if (tests == 0)
    {
        freeMemAllocsWithMessage(pM, "MEM-ALLOC-FAILED: tests\n");
        return;
    }

    char **testsReal = (char **)allocTestMem(pM, NUM_BENCH_HEADERS_ENTRIES, sizeof(char *));
    if (testsReal == 0)
    {
        freeMemAllocsWithMessage(pM, "MEM-ALLOC-FAILED: tests\n");
        return;
    }

    uint64_t *signatures = (uint64_t *)allocTestMem(pM, NUM_BENCH_HEADERS_ENTRIES, sizeof(uint64_t));
    if (signatures == 0)
    {
        freeMemAllocsWithMessage(pM, "MEM-ALLOC-FAILED: signatures\n");
        return;
    }

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
        char *pM = bench_header_names_lc(r);
        size_t l = strlen(pM);
        memcpy(p, pM, l);

        if (i != 0 && i % 34 == 0)
        {
            int r2 = rand();
            p[r2 % l] = 'X';
            expected[i] = NUM_HEADER_HANDLERS + 1;
        }
        else
        {
            expected[i] = r + 1;
        }

        size_t sIdx;
        uint64_t sig = 0;

        for (sIdx = 0; sIdx < l; sIdx++)
        {
            sig |= (0x1ul) << (p[sIdx] & 0x3F);
        }
        signatures[i] = sig;
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
        freeMemAllocs(pM);
        return;
    }

    size_t total = NUM_BENCH_HEADERS_ENTRIES * NUM_BENCH_HEADERS_RUNS;

    printf("CASES!#runs:%d|#tests:%d|#total:%d\n", (int)NUM_BENCH_HEADERS_RUNS, (int)NUM_BENCH_HEADERS_ENTRIES, (int)total);

#define NUM_BENCH_HEADERS_IMPLEMENTATIONS 7

    double times[8][NUM_BENCH_HEADERS_IMPLEMENTATIONS];
    char *names[NUM_BENCH_HEADERS_IMPLEMENTATIONS] = {"Norm", "IMP1", "IMP2", "IMP3", "IMP4", "Sorted", "IMP4b"};

    memset(times, 0, sizeof(double) * NUM_BENCH_HEADERS_IMPLEMENTATIONS * 8);

    size_t offset;

#define BENCH_HEADERS_OFFSET_LIMIT 8

    for (offset = 0; offset < BENCH_HEADERS_OFFSET_LIMIT; offset++)
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

            memset(buffer, 0, 32);
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

        end = clock();

        printf("Align:%d", (int)offset);
        printTimes("", start, end, total);
        printf("Hits: %d\n", (int)hits);
        //
        //
#if 1
        size_t testRun;
        printf("Normal...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_RUNS; testRun++)
        {
            int r;
            for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
            {
                char *p = testsReal[i];
                r = impl_headers_normal(p);
                if (r != expected[i])
                {
                    printf("FAILED: %d|%d|%s|(%d,%d)\n", (int)testRun, (int)i, testsReal[i], expected[i], r);
                    break;
                }
            }
        }

        end = clock();
        times[offset][0] = ((double)(end - start));
#endif
#if 1
        printf("IMP1...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_RUNS; testRun++)
        {
            int r;
            for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
            {
                r = impl_headers_aligned(testsReal[i]);
                if (r != expected[i])
                {
                    printf("FAILED: %d|%d|%s|(%d,%d)\n", (int)testRun, (int)i, testsReal[i], expected[i], r);
                    printf("LAST-SIZE:%d\n", (int)lastSize);
                    printf("T:%s|%s|\n", tests[i], testsReal[i]);
                    break;
                }
            }
        }

        end = clock();
        // printTimes("IMP1", start, end, total);
        times[offset][1] = ((double)(end - start));
#endif

#if 1
        printf("IMP2...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_RUNS; testRun++)
        {
            int r;
            for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
            {
                r = impl_headers_aligned2(testsReal[i]);
                if (r != expected[i])
                {
                    printf("FAILED: %d|%d|%s|(%d,%d)\n", (int)testRun, (int)i, testsReal[i], expected[i], r);
                    printf("LAST-SIZE:%d\n", (int)lastSize);
                    printf("T:%s|%s|\n", tests[i], testsReal[i]);
                    break;
                }
            }
        }

        end = clock();
        // printTimes("IMP1", start, end, total);
        times[offset][2] = ((double)(end - start));
#endif

#if 1
        printf("IMP3...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_RUNS; testRun++)
        {
            int r;
            for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
            {
                r = impl_headers_aligned3(testsReal[i]);
                if (r != expected[i])
                {
                    printf("FAILED: %d|%d|%s|(%d,%d)\n", (int)testRun, (int)i, testsReal[i], expected[i], r);
                    printf("LAST-SIZE:%d\n", (int)lastSize);
                    printf("T:%s|%s|\n", tests[i], testsReal[i]);
                    break;
                }
            }
        }

        end = clock();
        // printTimes("IMP1", start, end, total);
        times[offset][3] = ((double)(end - start));
#endif

#if 1
        printf("IMP4...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_RUNS; testRun++)
        {
            int r;
            for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
            {
                r = impl_headers_aligned4(testsReal[i], signatures[i]);
                if (r != expected[i])
                {
                    printf("FAILED: %d|%d|%s|(%d,%d)\n", (int)testRun, (int)i, testsReal[i], expected[i], r);
                    printf("LAST-SIZE:%d\n", (int)lastSize);
                    printf("T:%s|%s|\n", tests[i], testsReal[i]);
                    break;
                }
            }
        }

        end = clock();
        // printTimes("IMP1", start, end, total);
        times[offset][4] = ((double)(end - start));
#endif

#if HEADER_DATA_SORTED_NAMES
        printf("Sorted...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_RUNS; testRun++)
        {
            int r;
            for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
            {
                r = impl_headers_sorted_by_size(testsReal[i], signatures[i]);
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
        times[offset][5] = ((double)(end - start));
#endif
#if 1
        printf("IMP4b...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_RUNS; testRun++)
        {
            int r;
            for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
            {
                char *pathStart = testsReal[i];
                // first char valid?
                {
                    const uint64_t bit = (0x1ul) << ((*pathStart) & 0x3F);
                    if ((bit & HEADER_FIRST_CHAR_BITS_CONSTANT) == 0)
                    {
                        r = NUM_HEADER_HANDLERS + 1;
                    }
                    else
                    {
                        r = impl_headers_aligned4(pathStart, signatures[i]);
                    }
                }

                if (r != expected[i])
                {
                    printf("FAILED: %d|%d|%s|(%d,%d)\n", (int)testRun, (int)i, testsReal[i], expected[i], r);
                    printf("LAST-SIZE:%d\n", (int)lastSize);
                    printf("T:%s|%s|\n", tests[i], testsReal[i]);
                    break;
                }
            }
        }

        end = clock();
        times[offset][6] = ((double)(end - start));
#endif
    }

    uint64_t addr = (uint64_t)&bench_header_names[0];
    printf("Alignment: %d\n", (int)(addr & 0x7));

    printf("Offset\t");

    for (i = 0; i < NUM_BENCH_HEADERS_IMPLEMENTATIONS; i++)
    {
        printf("\t%12s", names[i]);
    }
    printf("\n");

    for (offset = 0; offset < BENCH_HEADERS_OFFSET_LIMIT; offset++)
    {
        printf("%6d\t", (int)offset);
        for (i = 0; i < NUM_BENCH_HEADERS_IMPLEMENTATIONS; i++)
        {
            double t = times[offset][i] / CLOCKS_PER_SEC;
            printf("\t%0.10f", t);
        }
        printf("\n");
    }
    printf("----\n");
    for (offset = 0; offset < BENCH_HEADERS_OFFSET_LIMIT; offset++)
    {
        printf("%5d+\t", (int)offset);
        for (i = 0; i < NUM_BENCH_HEADERS_IMPLEMENTATIONS; i++)
        {
            double t = perTestTime(times[offset][i], total);
            printf("\t%0.10f", t);
        }
        printf("\n");
    }
    printf("\n");

#undef BENCH_HEADERS_OFFSET_LIMIT

    printf("FREE TESTS!\n");
    for (i = 0; i < NUM_BENCH_HEADERS_ENTRIES; i++)
    {
        if (tests[i] != 0)
        {
            free(tests[i]);
        }
    }
    freeMemAllocs(pM);

    printf("----\n");
#undef NUM_BENCH_HEADERS_IMPLEMENTATIONS
}

#undef bench_header_names_lc
#undef bench_header_names
#undef bench_header_offsets
#undef bench_header_values
#undef bench_header_values
#undef bench_header_signatures

#if HEADER_DATA_SORTED_NAMES
#undef bench_header_names_sorted
#undef bench_header_offsets_sorted
#endif

#endif
