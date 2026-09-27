#ifndef HTTP_PARSER_BENCHMARK_SNIPPETS
#define HTTP_PARSER_BENCHMARK_SNIPPETS

#include <stdlib.h>
#include <time.h>
#include "test_string_buffer.h"
#include "http_parser_benchmark_util.h"

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

#if 0
#define NUM_BENCH_METHOD_ENTRIES 1000
#define NUM_BENCH_METHOD_RUNS 1000
#include "snippets/bench_method.h"

#define NUM_BENCH_PROTOCOL_ENTRIES 1000
#define NUM_BENCH_PROTOCOL_RUNS 100
#include "snippets/bench_protocol.h"

#define NUM_BENCH_TEXT_ENTRIES 100
#define NUM_BENCH_TEXT_RUNS 100
#include "snippets/bench_text.h"

#endif

#define NUM_BENCH_HEADERS_ENTRIES 1000
#define NUM_BENCH_HEADERS_RUNS 10
#include "snippets/bench_headers.h"

#define NUM_BENCH_HEADERS_PARSE_FIELD_ENTRIES 1000
#define NUM_BENCH_HEADERS_PARSE_FIELD_RUNS 10
#include "snippets/bench_headers_parse_field.h"

#endif