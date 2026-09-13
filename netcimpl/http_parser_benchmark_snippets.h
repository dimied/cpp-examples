#ifndef HTTP_PARSER_BENCHMARK_SNIPPETS
#define HTTP_PARSER_BENCHMARK_SNIPPETS

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

#include <stdlib.h>
#include <time.h>

#define NUM_BENCH_ENTRIES 10000
#define NUM_TEST_RUNS 50
#define TOTAL_TESTS (NUM_BENCH_ENTRIES * NUM_TEST_RUNS)

void print2()
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

void printTimes(char *name, clock_t start, clock_t end)
{
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    double perTest;
    if (TOTAL_TESTS < (0x1ul << 32))
    {
        double div = (double)TOTAL_TESTS;
        div *= (double)CLOCKS_PER_SEC;
        perTest = ((double)(end - start)) / div;
    }
    else
    {
        perTest = cpu_time_used / TOTAL_TESTS;
    }

    printf("%s: %f|%0.10f\n", name, cpu_time_used, perTest);
}

void bench_parse_method()
{
    print2();
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    int *results = (int *)calloc(NUM_BENCH_ENTRIES, sizeof(int));

    if (results != 0)
    {
        int *expected = (int *)calloc(NUM_BENCH_ENTRIES, sizeof(int));
        if (expected == 0)
        {
            free(results);
            return;
        }
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("RESULT! %f\n", cpu_time_used);

        char **tests = (char **)calloc(NUM_BENCH_ENTRIES, sizeof(char *));

        if (tests != 0)
        {
            end = clock();
            cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
            printf("TESTS! %f\n", cpu_time_used);
            size_t i;
            for (i = 0; i < NUM_BENCH_ENTRIES; i++)
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

            if (i == NUM_BENCH_ENTRIES)
            {

                printf("CASES!#runs:%d|#tests:%d|#total:%d\n", NUM_TEST_RUNS, NUM_BENCH_ENTRIES, TOTAL_TESTS);

                size_t testRun;
                printf("Normal...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_TEST_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_ENTRIES; i++)
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
                printTimes("Normal", start, end);

                printf("IMP1...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_TEST_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_ENTRIES; i++)
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
                printTimes("IMP1", start, end);

                printf("IMP2...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_TEST_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_ENTRIES; i++)
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
                printTimes("IMP2", start, end);

                printf("IMP3...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_TEST_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_ENTRIES; i++)
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
                printTimes("IMP3", start, end);
            }

            printf("FREE TESTS!\n");
            for (i = 0; i < NUM_BENCH_ENTRIES; i++)
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

int has, lastNI, lastMajor, lastMinor;

unsigned int impl_proto1(char *pathStart, int i, int recvRes)
{
    int pOK = 0;
    char *protStart = pathStart;

    while (pOK <= 10 && i < recvRes)
    {
        if (*pathStart <= '\n')
        {
            if (i > 1)
            {
                --pathStart;
                if (*pathStart != '\r')
                {
                    i = -i;
                }
            }
            else
            {
                i = -i;
            }

            break;
        }
        ++pOK;
        ++pathStart;
        ++i;
    }
    if (i < 0 || pOK < 8 || pOK > 10)
    {
#if 0
        lastMajor = pOK;
        lastMinor = i;
        lastNI = recvRes;
#endif
        // printf("CLOSE! %d|%d\n", i, pOK);
        //   closeCC(pCC);
        return 0;
    }

    int major, minor;
    *pathStart = 0;
    // TODO:
    //- test parsing vs. sscanf("HTTP/x.y"), sscanf("/x.y")
    //- test with different alignments
    int numItems = sscanf(protStart, "HTTP/%d.%d", &major, &minor);
    *pathStart = '\r';
    // printf("PROT:%s|M=%d,m=%d\n", protStart, major, minor);
    if (numItems != 2 || major < 0 || major > 2 || minor < 0 || minor > 9)
    {
#if 0
    lastNI = numItems;
    lastMajor = major;
    lastMinor = minor;
#endif
        // printf("FAIL:%s|\n", protStart);
        //  closeCC(pCC);
        return 0;
    }
    minor |= major << 4;
    // printf("M: %d\n", minor);
    // pathStart += 2;

    return ((unsigned int)minor << 8);
    //++i;
}

#define START_PROT WORD8_TO_UINT64('H', 'T', 'T', 'P', '/', '0', '.', '9')
#define START_END WORD8_TO_UINT64('H', 'T', 'T', 'P', '/', '2', '.', '0')

uint64_t protoValues[4] = {
    WORD8_TO_UINT64('H', 'T', 'T', 'P', '/', '0', '.', '9'),
    WORD8_TO_UINT64('H', 'T', 'T', 'P', '/', '1', '.', '0'),
    WORD8_TO_UINT64('H', 'T', 'T', 'P', '/', '1', '.', '1'),
    WORD8_TO_UINT64('H', 'T', 'T', 'P', '/', '2', '.', '0'),
};

unsigned int impl_proto2(char *pathStart)
{
    uint64_t v = (uint64_t)pathStart;

    if ((v & 0x7) == 0)
    {
        v = *((uint64_t *)pathStart);
        pathStart += 8;
    }
    else
    {
        v = (uint64_t)(*pathStart);
        ++pathStart;
        int s = 8;

        for (size_t i = 0; i < 7; i++)
        {
            v = (*pathStart) << s;
            s += 8;
            ++pathStart;
        }
    }

    unsigned int retVal = (0x09 | (0x10 << 8) | (0x11 << 16) | (0x20 << 24));
    int i;
    for (i = 0; i < 4; i++)
    {
        if (v == protoValues[i])
        {
            i = (retVal & 0xFF) << 8;
            break;
        }
        retVal >>= 8;
    }

    if (i > 4 && (*pathStart == '\r') && (pathStart[1] == '\n'))
    {
        return i;
    }
    return 0;
}

unsigned int impl_proto3(char *pathStart)
{
    uint64_t v = (uint64_t)pathStart;

    if ((v & 0x7) == 0)
    {
        v = *((uint64_t *)pathStart);
        unsigned int retVal = (0x09 | (0x10 << 8) | (0x11 << 16) | (0x20 << 24));
        int i;
        for (i = 0; i < 4; i++)
        {
            if (v == protoValues[i])
            {
                i = (retVal & 0xFF) << 8;
                break;
            }
            retVal >>= 8;
        }

        if (i > 4)
        {
            pathStart += 8;
            v = *((uint64_t *)pathStart);
            v &= 0xFFFFul;
            if (v == ('\r' | (('\n') << 8)))
            {
                return i;
            }
        }
        return 0;
    }

    v = (uint64_t)(*pathStart);
    ++pathStart;
    int s = 8;

    for (size_t i = 0; i < 7; i++)
    {
        v = (*pathStart) << s;
        s += 8;
        ++pathStart;
    }

    unsigned int retVal = (0x09 | (0x10 << 8) | (0x11 << 16) | (0x20 << 24));
    int i;
    for (i = 0; i < 4; i++)
    {
        if (v == protoValues[i])
        {
            i = (retVal & 0xFF) << 8;
            break;
        }
        retVal >>= 8;
    }

    if (i > 4 && (*pathStart == '\r') && (pathStart[1] == '\n'))
    {
        return i;
    }
    return 0;
}

#define P_VAL WORD8_TO_UINT64('H', 'T', 'T', 'P', '/', 0, '.', 0)
#define P_MASK WORD8_TO_UINT64(0xFFul, 0xFFul, 0xFFul, 0xFFul, 0xFFul, 0, 0xFFul, 0)
#define P_MASK2 WORD8_TO_UINT64('9', '0', '0', '1', '1', '1', '0', '2')

unsigned int impl_proto4(char *pathStart)
{
    uint64_t v = (uint64_t)pathStart;

    if ((v & 0x7) == 0)
    {
        v = *((uint64_t *)pathStart);

        if ((v & P_MASK) == P_VAL)
        {
            v = ((v >> 32) & 0xFF00) | (v >> 56);

#if 1
            uint64_t vv = P_MASK2;
            unsigned int retVal = (0x09 | (0x10 << 8) | (0x11 << 16) | (0x20 << 24));

            int i;
            for (i = 0; i < 4; i++)
            {
                if (v == (vv & 0xFFFF))
                {
                    i = (retVal & 0xFF) << 8;
                    break;
                }
                vv >>= 16;
                retVal >>= 8;
            }
#else
            uint64_t vv = P_MASK2;
            // unsigned int retVal = (0x09 | (0x10 << 8) | (0x11 << 16) | (0x20 << 24));

            int i;
            for (i = 0; i < 4; i++)
            {
                if (v == (vv & 0xFFFF))
                {
                    i = (vv & 0xFFFF) - ('0' | ('0' << 8));
                    i = (i & 0xF) | (i >> 4);
                    i <<= 8;
                    break;
                }
                vv >>= 16;
                // retVal >>= 8;
            }
#endif
            if (i > 4)
            {
                pathStart += 8;
                v = *((uint64_t *)pathStart);
                v &= 0xFFFFul;
                if (v == ('\r' | (('\n') << 8)))
                {
                    return i;
                }
            }
        }
        return 0;
    }

    v = (uint64_t)(*pathStart);
    ++pathStart;
    int s = 8;

    for (size_t i = 0; i < 7; i++)
    {
        v = (*pathStart) << s;
        s += 8;
        ++pathStart;
    }

    unsigned int retVal = (0x09 | (0x10 << 8) | (0x11 << 16) | (0x20 << 24));
    int i;
    for (i = 0; i < 4; i++)
    {
        if (v == protoValues[i])
        {
            i = (retVal & 0xFF) << 8;
            break;
        }
        retVal >>= 8;
    }

    if (i > 4 && (*pathStart == '\r') && (pathStart[1] == '\n'))
    {
        return i;
    }
    return 0;
}

typedef struct _PT
{
    char *str;
    unsigned int ret_value;
} PT;

#define PROTO_TEST(S, RV) {S, RV}

PT protocolTests[] = {
    PROTO_TEST("HTTP", 0),
    PROTO_TEST("HTTP/", 0),
    PROTO_TEST("HTTP/\r", 0),
    PROTO_TEST("HTTP/\r\n", 0),
    PROTO_TEST("HTTP/0", 0),
    PROTO_TEST("HTTP/0\r\n", 0),
    PROTO_TEST("HTTP/0.9\r\n", 0x0900),
    PROTO_TEST("HTTP/1.0\r\n", 0x1000),
    PROTO_TEST("HTTP/1.1\r\n", 0x1100),
    PROTO_TEST("HTTP/2.0\r\n", 0x2000),
};

#define NPT (sizeof(protocolTests) / sizeof(protocolTests[0]))

#define NUM_BENCH_ENTRIES 10000
#define NUM_TEST_RUNS 50
#define TOTAL_TESTS (NUM_BENCH_ENTRIES * NUM_TEST_RUNS)

void bench_parse_protocol()
{
    printf("----\n");
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    int *results = (int *)calloc(NUM_BENCH_ENTRIES, sizeof(int));

    if (results != 0)
    {
        int *expected = (int *)calloc(NUM_BENCH_ENTRIES, sizeof(int));
        if (expected == 0)
        {
            free(results);
            return;
        }
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("RESULT! %f\n", cpu_time_used);

        char **tests = (char **)calloc(NUM_BENCH_ENTRIES, sizeof(char *));

        if (tests != 0)
        {
            end = clock();
            cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
            printf("TESTS! %f\n", cpu_time_used);
            size_t i;
            for (i = 0; i < NUM_BENCH_ENTRIES; i++)
            {
                char *p = (char *)calloc(1, 32);
                if (p == 0)
                {
                    break;
                }
                // different strings
                int r = rand();
                r %= NPT;
                char *pM = protocolTests[r].str;
                int rv = protocolTests[r].ret_value;
                size_t l = strlen(pM);
                memcpy(p, pM, l);
                // printf("T:%s|%x\n", pM, rv);

                if (i != 0 && i % 34 == 0)
                {
                    int r2 = rand();
                    p[r2 % l] = 'X';
                    expected[i] = 0;
                }
                else
                {
                    expected[i] = rv;
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

            if (i == NUM_BENCH_ENTRIES)
            {

                printf("CASES!#runs:%d|#tests:%d|#total:%d\n", NUM_TEST_RUNS, NUM_BENCH_ENTRIES, TOTAL_TESTS);

                size_t testRun;
                printf("Normal...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_TEST_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_ENTRIES; i++)
                    {
                        char *p = tests[i];
                        int r = impl_proto1(p, 1, 24);
                        if (r != expected[i])
                        {
                            printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, tests[i], expected[i], r);
                            printf("Last:%d| NI=%d|MAJOR=%d|MINOR=%d\n", has, lastNI, lastMajor, lastMinor);
                            break;
                        }
                    }
                }

                end = clock();
                printTimes("Normal", start, end);
#if 1
                printf("IMP1...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_TEST_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_ENTRIES; i++)
                    {
                        int r = impl_proto2(tests[i]);
                        if (r != expected[i])
                        {
                            printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, tests[i], expected[i], r);
                            break;
                        }
                    }
                }

                end = clock();
                printTimes("IMP1", start, end);
#endif
#if 1
                printf("IMP2...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_TEST_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_ENTRIES; i++)
                    {
                        int r = impl_proto3(tests[i]);
                        if (r != expected[i])
                        {
                            printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, tests[i], expected[i], r);
                            break;
                        }
                    }
                }

                end = clock();
                printTimes("IMP2", start, end);
#endif
#if 1
                printf("IMP3...\n");
                start = clock();

                for (testRun = 0; testRun < NUM_TEST_RUNS; testRun++)
                {
                    for (i = 0; i < NUM_BENCH_ENTRIES; i++)
                    {
                        int r = impl_proto4(tests[i]);
                        if (r != expected[i])
                        {
                            printf("FAILED: %ld|%ld|%s|(%d,%d)\n", testRun, i, tests[i], expected[i], r);
                            break;
                        }
                    }
                }

                end = clock();
                printTimes("IMP3", start, end);
#endif
            }

            printf("FREE TESTS!\n");
            for (i = 0; i < NUM_BENCH_ENTRIES; i++)
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
    printf("----\n");
}
#endif