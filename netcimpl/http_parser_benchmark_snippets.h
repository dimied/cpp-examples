#ifndef HTTP_PARSER_BENCHMARK_SNIPPETS
#define HTTP_PARSER_BENCHMARK_SNIPPETS

#include <stdlib.h>
#include <time.h>
#include "test_string_buffer.h"

double perTestTime(double diff, size_t total_tests)
{

    // we can store upto 2^53 precisely in a double
    uint64_t limit = ((0x1ul << 53) / CLOCKS_PER_SEC) + 10;

    if (total_tests < limit)
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


#define BENCH_FREE_MEM(M) \
    size_t t = 0;         \
    do                    \
    {                     \
        if (M[t] != 0)    \
        {                 \
            free(M[t]);   \
        }                 \
        t++;              \
    } while (t + 1 < (sizeof(M) / sizeof(M[0])))

#define NUM_BENCH_METHOD_ENTRIES 1000
#define NUM_BENCH_METHOD_RUNS 1000
#include "snippets/bench_method.h"

#define NUM_BENCH_PROTOCOL_ENTRIES 1000
#define NUM_BENCH_PROTOCOL_RUNS 100
#include "snippets/bench_protocol.h"

#define NUM_BENCH_TEXT_ENTRIES 100
#define NUM_BENCH_TEXT_RUNS 100
#include "snippets/bench_text.h"

#define NUM_BENCH_HEADERS_ENTRIES 100
#define NUM_BENCH_HEADERS_RUNS 1
#include "snippets/bench_headers.h"

#endif