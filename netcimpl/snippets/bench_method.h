#ifndef BENCH_METHOD_SNIPPETS
#define BENCH_METHOD_SNIPPETS

void printHttpMethods2()
{
    union
    {
        uint64_t v64[2];
        char v8[16];
    } vals;

    vals.v64[0] = 0;
    vals.v64[1] = 0;
    int j;
    printf("V64:");
    for (j = 0; j < NUM_HTTP_METHODS; j++)
    {
        uint64_t v = HttpMethods2[j];
        vals.v64[0] = v;
        printf("%s|", vals.v8);
    }
    printf("\n");
}

int impl_parse_method1(char *buffer, char *method, int i, int recvRes)
{
    int s = 0;

    // parse method
    while (i < 9 && i < recvRes)
    {
        // invalid chars or '\r', '\n'
        if (*method < ' ')
        {
            --s;
            break;
        }
        if (*method == ' ')
        {
            ++s;
            break;
        }
        ++method;
        ++i;
    }

    // line-end found?
    if (s <= 0)
    {
        return RECV_PARSE_ERROR_METHOD_FAILED;
    }
    else
    {
        *method = 0;
    }

    int j;
    {
        //--i;
        for (j = 0; j < NUM_HTTP_METHODS; j++)
        {
            if (0 == strcmp(HttpMethods[j], buffer))
            {
                break;
            }
        }
        // i += 2;
    }
    *method = ' ';
    return j;
}

#define METHOD_SHIFT_CONST ((0 | (8 << 7) | (32 << 14) | (24 << 21)) | (16ul << 35) | (24ul << 42) | (32ul << 49) | (16ul << 56))

int impl_parse_method2(uint64_t method)
{
    uint64_t shift = METHOD_SHIFT_CONST;
    int j;

    for (j = 0; j < NUM_HTTP_METHODS; j++)
    {
        uint64_t m = shift & 0x7F;
        m = (UINT64_MAX) >> m;
        m &= method;

        if (m == HttpMethods2[j])
        {
            break;
        }

        shift >>= 7;
    }
    return j;
}

uint64_t masks[NUM_HTTP_METHODS] = {
    (UINT64_MAX),
    (UINT64_MAX >> 8),
    (UINT64_MAX >> 32),
    (UINT64_MAX >> 24),
    (UINT64_MAX),
    (UINT64_MAX >> 16),
    (UINT64_MAX >> 24),
    (UINT64_MAX >> 32),
    (UINT64_MAX >> 16)};

int impl_parse_method23(uint64_t method)
{
    int j;

    for (j = 0; j < NUM_HTTP_METHODS; j++)
    {
        uint64_t m = method & masks[j];

        if (m == HttpMethods2[j])
        {
            break;
        }
    }
    return j;
}

#define METHOD_SHIFT_CONST2 ((0 | (1 << 4) | (4 << 8) | (3 << 12)) | (2 << 20) | (3 << 24) | (4ul << 28) | (2ul << 32))

int impl_parse_method22(uint64_t method)
{
    uint64_t shift = METHOD_SHIFT_CONST2;
    int j;

    for (j = 0; j < NUM_HTTP_METHODS; j++)
    {
        uint64_t m = shift & 0xF;
        m <<= 3; // x8
        m = (UINT64_MAX) >> m;
        m &= method;

        if (m == HttpMethods2[j])
        {
            break;
        }

        shift >>= 4;
    }
    return j;
}

void bench_parse_method()
{
    printHttpMethods2();
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    int *results = (int *)calloc(NUM_BENCH_METHOD_ENTRIES, sizeof(int));

    if (results != 0)
    {
        int *expected = (int *)calloc(NUM_BENCH_METHOD_ENTRIES, sizeof(int));
        if (expected == 0)
        {
            free(results);
            return;
        }
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("RESULT! %f\n", cpu_time_used);

        char **tests = (char **)calloc(NUM_BENCH_METHOD_ENTRIES, sizeof(char *));

        if (tests != 0)
        {
            end = clock();
            cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
            printf("TESTS! %f\n", cpu_time_used);
            size_t i;
            for (i = 0; i < NUM_BENCH_METHOD_ENTRIES; i++)
            {
                char *p = (char *)calloc(1, 16);
                if (p == 0)
                {
                    break;
                }
                // different strings
                int r = rand();
                r %= NUM_HTTP_METHODS;
                char *pM = HttpMethods[r];
                size_t l = strlen(pM);
                memcpy(p, pM, l);
                p[l] = ' ';

                if (i != 0 && i % 34 == 0)
                {
                    int r2 = rand();
                    p[r2 % l] = 'X';
                    expected[i] = NUM_HTTP_METHODS;
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
            end = clock();
            cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
            printf("TESTS GENERATED! %f\n", cpu_time_used);

            if (i == NUM_BENCH_METHOD_ENTRIES)
            {
                size_t total = NUM_BENCH_METHOD_RUNS * NUM_BENCH_METHOD_ENTRIES;
                
                printf("CASES!#runs:%d|#tests:%d|#total:%ld\n", NUM_BENCH_METHOD_RUNS, NUM_BENCH_METHOD_ENTRIES, total);
                size_t testRun;

                printf("Normal...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_BENCH_METHOD_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_METHOD_ENTRIES; i++)
                    {
                        char *p = tests[i];
                        int r = impl_parse_method1(p, p, 1, 12);
                        if (r != expected[i])
                        {
                            printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, tests[i], expected[i], r);
                            break;
                        }
                    }
                }

                end = clock();
                printTimes("Normal", start, end, total);

                printf("IMP1...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_BENCH_METHOD_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_METHOD_ENTRIES; i++)
                    {
                        uint64_t *p = (uint64_t *)tests[i];

                        int r = impl_parse_method2(*p);
                        if (r != expected[i])
                        {
                            printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, tests[i], expected[i], r);
                            break;
                        }
                    }
                }

                end = clock();
                printTimes("IMP1", start, end, total);

                printf("IMP2...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_BENCH_METHOD_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_METHOD_ENTRIES; i++)
                    {
                        uint64_t *p = (uint64_t *)tests[i];

                        int r = impl_parse_method22(*p);
                        if (r != expected[i])
                        {
                            printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, tests[i], expected[i], r);
                            break;
                        }
                    }
                }

                end = clock();
                printTimes("IMP2", start, end, total);

                printf("IMP3...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_BENCH_METHOD_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_METHOD_ENTRIES; i++)
                    {
                        uint64_t *p = (uint64_t *)tests[i];

                        int r = impl_parse_method23(*p);
                        if (r != expected[i])
                        {
                            printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, tests[i], expected[i], r);
                            break;
                        }
                    }
                }

                end = clock();
                printTimes("IMP3", start, end, total);
            }

            printf("FREE TESTS!\n");
            for (i = 0; i < NUM_BENCH_METHOD_ENTRIES; i++)
            {
                if (tests[i] != 0)
                {
                    free(tests[i]);
                }
            }
            free(tests);
        }
        printf("FREE RESULT!\n");
        free(expected);
        free(results);
    }
}

#endif