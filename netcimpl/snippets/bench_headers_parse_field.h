#ifndef BENCH_HEADERS_PARSE_FIELD_SNIPPETS
#define BENCH_HEADERS_PARSE_FIELD_SNIPPETS

#include <string.h>

#define bench_header_names_lc(idx) (header_comp_data.header_names_lowercase[idx])

#define BENCH_HEADERS_PARSE_FIELD_RES_OK 0
#define BENCH_HEADERS_PARSE_FIELD_RES_NEG 1
#define BENCH_HEADERS_PARSE_FIELD_RES_INVALID_CHAR 1
/*
message-header = field-name ":" [ field-value ]
field-name     = token
field-value    = *( field-content | LWS )
field-content  = <the OCTETs making up the field-value and consisting of
                  either *TEXT or combinations of token, separators, and
                  quoted-string>

CR             = <US-ASCII CR, carriage return (13)>
LF             = <US-ASCII LF, linefeed (10)>
SP             = <US-ASCII SP, space (32)>
HT             = <US-ASCII HT, horizontal-tab (9)>
CRLF           = CR LF
LWS            = [CRLF] 1*( SP | HT )
OCTET          = <any 8-bit sequence of data>
CHAR           = <any US-ASCII character (octets 0 - 127)>
CTL            = <any US-ASCII control character (octets 0 - 31) and DEL (127)>
TEXT           = <any OCTET except CTLs, but including LWS>

token          = 1*<any CHAR except CTLs or separators>
separators     = "(" | ")" | "<" | ">" | "@" | "," | ";" | ":" | "\"
               | <"> | "/" | "[" | "]" | "?" | "=" | "{" | "}" | SP | HT

quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
qdtext         = <any TEXT except <">>
quoted-pair    = "\" CHAR

*/

#define INVALID_FIELD_NAME_CHARS "()<>@,;:\\\"/[]?={} \t"

#define BENCH_HEADERS_PARSE_FIELD_MEM_SIZE 48

size_t generate_test_data_for_bench_parse_headers_parse_field(TestAlignedText **tests, int *expected)
{
    size_t i;
    int generate_error;
    int states[4] = {0, 0, 0, 0};

    for (i = 0; i < NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES; i++)
    {
        TestAlignedText *pAT = allocTestAlignedText(BENCH_HEADERS_PARSE_FIELD_MEM_SIZE, &generate_error);
        if (pAT == 0)
        {
            printf("GENERATE-ERROR: %d\n", generate_error);
            break;
        }

        // different strings
        int r = rand();

        r %= NUM_HEADER_HANDLERS;
        char *pTestString = bench_header_names_lc(r);

        size_t l = strlen(pTestString);
        generate_error = setTestAlignedText(pAT, pTestString, l, 2);

        if (generate_error != TEST_ALIGNED_TEXT_OK)
        {
            printf("GENERATE-ERROR:SET: %d\n", generate_error);
            break;
        }
        char *pStr = get_TestAlignedText(pAT);
        pStr[l] = ':';

        if (i != 0 && l > 3)
        {
            int r2 = rand();
            r2 %= 100;
            int r3 = rand();

            // int charIdx =

            if (r2 < 25)
            {
                pStr[r3 % (l - 1)] = 'Z';
                expected[i] = BENCH_HEADERS_PARSE_FIELD_RES_OK;
                states[0]++;
            }
            else if (r2 < 50)
            {
                pStr[r3 % (l - 1)] = -1;
                expected[i] = BENCH_HEADERS_PARSE_FIELD_RES_NEG;
                states[1]++;
            }
            else if (r2 < 75)
            {
                pStr[r3 % (l - 1)] = 20;
                expected[i] = BENCH_HEADERS_PARSE_FIELD_RES_INVALID_CHAR;
                states[2]++;
            }
            else
            {
                pStr[r3 % (l - 1)] = 'Z' + 1;
                expected[i] = BENCH_HEADERS_PARSE_FIELD_RES_INVALID_CHAR;
                states[2]++;
            }
        }
        else
        {
            expected[i] = BENCH_HEADERS_PARSE_FIELD_RES_OK;
            states[0]++;
        }
#if 0
                if (i < 10)
                {
                    printf("M:%ld|%s|\n", i, p);
                }
#endif
        tests[i] = pAT;
    }

    printf("GEN:stats: #ok=%d | #neg=%d | #inv=%d |\n", states[0], states[1], states[2]);
    return i;
}

static char invalid_chars[] = INVALID_FIELD_NAME_CHARS;

#define INV_CHAR_LEN (sizeof(INVALID_FIELD_NAME_CHARS) - 1)


void bench_parse_headers_parse_field()
{
    printf("----\n");

    double cpu_time_used;
    TestMemAllocations *pM = testMemAllocs("bench_headers_parse_field");

    if (pM == 0)
    {
        printf("MEM-ALLOC-FAILED: base\n");
        return;
    }

    clock_t end, start = clock();

    int *results = (int *)allocTestMem(pM, NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES, sizeof(int));
    if (results == 0)
    {
        printf("MEM-ALLOC-FAILED: results\n");
        return;
    }

    int *expected = (int *)allocTestMem(pM, NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES, sizeof(int));
    if (expected == 0)
    {
        freeMemAllocsWithMessage(pM, "MEM-ALLOC-FAILED: expected\n");
        return;
    }

    TestAlignedText **tests = (TestAlignedText **)calloc(NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES, sizeof(TestAlignedText *));
    if (tests == 0)
    {
        freeMemAllocsWithMessage(pM, "MEM-ALLOC-FAILED: tests\n");
        return;
    }

    char **testsReal = (char **)allocTestMem(pM, NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES, sizeof(char *));
    if (testsReal == 0)
    {
        free(tests);
        freeMemAllocsWithMessage(pM, "MEM-ALLOC-FAILED: tests\n");
        return;
    }

    uint64_t *signatures = (uint64_t *)allocTestMem(pM, NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES, sizeof(uint64_t));
    if (signatures == 0)
    {
        free(tests);
        freeMemAllocsWithMessage(pM, "MEM-ALLOC-FAILED: signatures\n");
        return;
    }

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("MEMORY! %f\n", cpu_time_used);

    start = clock();

    size_t generated = generate_test_data_for_bench_parse_headers_parse_field(tests, expected);

    memcpy(testsReal, tests, NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES * sizeof(char *));

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("TESTS! %f\n", cpu_time_used);

    if (generated != NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES)
    {
        while (generated > 0)
        {
            if (tests[generated] != 0)
            {
                freeTestAlignedText(tests[generated]);
            }
        }
        printf("MEM-ALLOC-FAILED: tests generation\n");
        free(tests);
        freeMemAllocs(pM);
        return;
    }

    size_t total = NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES * NUM_BENCH_HEADERS_PARSE_FIELD_RUNS;

    printf("CASES!#runs:%d|#tests:%d|#total:%ld\n", NUM_BENCH_HEADERS_PARSE_FIELD_RUNS, NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES, total);

#define NUM_BENCH_HEADERS_PARSE_FIELD_IMPLEMENTATIONS 2

    double times[8][NUM_BENCH_HEADERS_PARSE_FIELD_IMPLEMENTATIONS];
    char *names[NUM_BENCH_HEADERS_PARSE_FIELD_IMPLEMENTATIONS] = {"Norm", "IMP1"};

    memset(times, 0, sizeof(double) * NUM_BENCH_HEADERS_PARSE_FIELD_IMPLEMENTATIONS * 8);

    size_t offset, i;

#define BENCH_HEADERS_OFFSET_LIMIT 8

    for (offset = 0; offset < BENCH_HEADERS_OFFSET_LIMIT; offset++)
    {
        // align values
        start = clock();
        int err;
        for (i = 0; i < NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES; i++)
        {
            err = realign_TestAlignedText(tests[i], offset);
            if (err != TEST_ALIGNED_TEXT_OK)
            {
                break;
            }
            testsReal[i] = get_TestAlignedText(tests[i]);
        }
        if (err != TEST_ALIGNED_TEXT_OK)
        {
            printf("Realign: failed: %ld | ERROR: %d\n", i, err);
            break;
        }

        end = clock();

        printf("Align:%ld", offset);
        printTimes("", start, end, total);
        // printf("Hits: %ld\n", hits);
        //
        //

#if 1
        size_t testRun;

        printf("Normal...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_PARSE_FIELD_RUNS; testRun++)
        {
            int r;
            for (i = 0; i < NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES; i++)
            {
                char *pTest = testsReal[i];
                r = BENCH_HEADERS_PARSE_FIELD_RES_OK;
                size_t j;

                while (*pTest != 0)
                {
                    char c = *pTest;

                    if (c < 0 || c == 127)
                    {
                        r = BENCH_HEADERS_PARSE_FIELD_RES_NEG;
                        // printf("NEG-FOUND:%ld|%c\n", i, c);
                        break;
                    }
                    if (c < 33)
                    {
                        r = BENCH_HEADERS_PARSE_FIELD_RES_INVALID_CHAR;
                        // printf("INV-FOUND:%ld|%c\n", i, c);
                        break;
                    }

                    if (c == ':')
                    {
                        break;
                    }
                    for (j = 0; j < INV_CHAR_LEN; j++)
                    {
                        if (c == invalid_chars[j])
                        {
                            break;
                        }
                    }
                    if (j < INV_CHAR_LEN)
                    {
                        r = BENCH_HEADERS_PARSE_FIELD_RES_INVALID_CHAR;
                        break;
                    }
#if 0
                    if (c >= 'A' && c <= 'Z')
                    {
                        *pTest = c + 32;
                    }
#endif
                    ++pTest;
                }
#if 1
                if (r != expected[i])
                {
                    printf("FAILED: %ld|%ld|%s|(exp=%d,got=%d)\n", testRun, i, testsReal[i], expected[i], r);
                    break;
                }
#endif
            }
        }

        end = clock();
        times[offset][0] = ((double)(end - start));
#endif
#if 1
        printf("IMP1...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_HEADERS_PARSE_FIELD_RUNS; testRun++)
        {
            int r;
            for (i = 0; i < NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES; i++)
            {
                char *pTest = testsReal[i];

                r = BENCH_HEADERS_PARSE_FIELD_RES_OK;

                uint64_t v;
                int shift;

                while (*pTest != 0)
                {
                    char c = *pTest;
                    if (c < 33)
                    {
                        r = BENCH_HEADERS_PARSE_FIELD_RES_NEG;
                        // printf("NEG-FOUND:%ld|%c|%d\n", i, c, c);
                        break;
                    }

                    shift = (c & 0x3F);
                    v = (0x1ul) << shift;
                    char off = (c & 0x40);

                    if (off != 0)
                    {
                        v &= HEADER_FIELD_NAME_VALID_CHARS2_BITS;
                        // offset 32, if v valid
                        off = (HEADER_FIELD_NAME_LOWERCASE_OFFSET_BITS >> shift) & 0x20;
                    }
                    else
                    {
                        // no lower case for values 0-64
                        v &= HEADER_FIELD_NAME_VALID_CHARS1_BITS;
                        off = 0;
                    }

                    if (v)
                    {
                        if (c == ':')
                        {
                            break;
                        }
                        // if offset, add it.
                        // c |= (off>>1);
                    }
                    else
                    {
                        r = BENCH_HEADERS_PARSE_FIELD_RES_INVALID_CHAR;
                        break;
                    }

                    ++pTest;
                }
#if 1
                if (r != expected[i])
                {
                    printf("FAILED: %ld|%ld|%s|(exp=%d,got=%d)\n", testRun, i, testsReal[i], expected[i], r);
                    break;
                }
#endif
            }
        }

        end = clock();
        // printTimes("IMP1", start, end, total);
        times[offset][1] = ((double)(end - start));
#endif
    }

    printf("Offset\t");

    for (i = 0; i < NUM_BENCH_HEADERS_PARSE_FIELD_IMPLEMENTATIONS; i++)
    {
        printf("\t%12s", names[i]);
    }
    printf("\n");

    for (offset = 0; offset < BENCH_HEADERS_OFFSET_LIMIT; offset++)
    {
        printf("%6ld\t", offset);
        for (i = 0; i < NUM_BENCH_HEADERS_PARSE_FIELD_IMPLEMENTATIONS; i++)
        {
            double t = times[offset][i] / CLOCKS_PER_SEC;
            printf("\t%0.10f", t);
        }
        printf("\n");
    }
    printf("----\n");
    for (offset = 0; offset < BENCH_HEADERS_OFFSET_LIMIT; offset++)
    {
        printf("%5ld+\t", offset);
        for (i = 0; i < NUM_BENCH_HEADERS_PARSE_FIELD_IMPLEMENTATIONS; i++)
        {
            double t = perTestTime(times[offset][i], total);
            printf("\t%0.10f", t);
        }
        printf("\n");
    }
    printf("\n");

#undef BENCH_HEADERS_OFFSET_LIMIT

    printf("FREE TESTS!\n");
    for (i = 0; i < NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES; i++)
    {
        if (tests[i] != 0)
        {
            freeTestAlignedText(tests[i]);
        }
    }
    free(tests);
    freeMemAllocs(pM);

    printf("----\n");
#undef NUM_BENCH_HEADERS_PARSE_FIELD_IMPLEMENTATIONS
}

#undef bench_header_names_lc

#endif
