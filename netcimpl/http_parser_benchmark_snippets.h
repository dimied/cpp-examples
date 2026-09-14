#ifndef HTTP_PARSER_BENCHMARK_SNIPPETS
#define HTTP_PARSER_BENCHMARK_SNIPPETS

#include <stdlib.h>
#include <time.h>

void printTimes(char *name, clock_t start, clock_t end, size_t total_tests)
{
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    double perTest;
    // we can store upto 2^53 precisely in a double
    uint64_t limit = ((0x1ul << 53) / CLOCKS_PER_SEC) + 10;

    if (total_tests < limit)
    {
        double div = (double)total_tests;
        div *= (double)CLOCKS_PER_SEC;
        perTest = ((double)(end - start)) / div;
    }
    else
    {
        perTest = cpu_time_used / (double)total_tests;
    }

    printf("%s: %f|%0.10f\n", name, cpu_time_used, perTest);
}

#define NUM_BENCH_METHOD_ENTRIES 1000
#define NUM_BENCH_METHOD_RUNS 1000
#include "snippets/bench_method.h"

#define NUM_BENCH_PROTOCOL_ENTRIES 1000
#define NUM_BENCH_PROTOCOL_RUNS 1000
#include "snippets/bench_protocol.h"

#endif