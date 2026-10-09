
#include "base64.h"

#ifndef ADD_TO_SB
#define ADD_TO_SB(B, FORMAT, ...)                                 \
    do                                                            \
    {                                                             \
        char temp_buffer[64];                                     \
        memset(temp_buffer, 0, sizeof(temp_buffer));              \
        int temp_len = sprintf(temp_buffer, FORMAT, __VA_ARGS__); \
        addToTestStringBuffer(&B, temp_buffer, temp_len);         \
    } while (0)
#endif

#include "tinf.h"

#include "gzip.h"

#include "../print_util.h"

unsigned char dec[128];

unsigned char no_compression[] = {0x4, 0x4, 0x0, 0xFB, 0xFF, 0x21, 0x56, 0x76, 0xF2};
unsigned char no_compression_result[4];

char *b = "eJzzSM3JyVcoyUgFABBxA1Y=";

CompressionResult compRes;

unsigned char gz_result[200];

struct gzip_TestCase
{
    const char *in_file;
    const char *test_file;
};

#define GZIP_TEST_CASE(INFILE, TESTFILE) {INFILE, TESTFILE}

struct gzip_TestCase gzip_test_cases[] = {
    GZIP_TEST_CASE("testdata/ddd.txt.gz", "testdata/ddd.txt"),
    GZIP_TEST_CASE("testdata/html1.html.gz", "testdata/html1.html"),
};

#define GZIP_TEST_CASE_RESULT_SIZE (0x1 << 20)

unsigned char gzip_tc_result[GZIP_TEST_CASE_RESULT_SIZE];

void prepare_comp_res(void *data, int gzSize)
{
    memset(&compRes, 0, sizeof(compRes));
    memset(gzip_tc_result, 0, GZIP_TEST_CASE_RESULT_SIZE);
    memset(gzip_buffer, 0, sizeof(gzip_buffer));

    compRes.in_data = data;
    compRes.in_size = gzSize;
    compRes.out_data = gzip_tc_result;
    compRes.out_size = GZIP_TEST_CASE_RESULT_SIZE;

    inflate_dest_size = 0;
}

TestStringBuffer tb_steps;

void test_gzip()
{

    size_t num_gzip_tc = sizeof(gzip_test_cases) / sizeof(gzip_test_cases[0]);
    size_t gzSize;
    size_t tcIdx = 1;
    num_gzip_tc = 2;
    const int bytes_per_line = 32;

    printf("ID:sz:%d", (int)sizeof(struct inflate_state));
    printf(" | TREE:sz:%d\n", (int)sizeof(struct inflate_tree));

    clearTestStringBuffer(&b1);
    clearTestStringBuffer(&b2);
    clearTestStringBuffer(&tb_steps);

    INFLATE_ALGO_CLEAR_STEPS;

    for (; tcIdx < num_gzip_tc; tcIdx++)
    {
        const char *pInfile = gzip_test_cases[tcIdx].in_file;
        printf("+TC:%d|%s|", (int)tcIdx, pInfile);

        gzSize = 0;

        void *data = readFile(pInfile, &gzSize);

        if (data != 0)
        {
            initTestStringBuffer(&b1, 20000);
            initTestStringBuffer(&b2, 20000);

            printf("gz-size:%d|\n", (int)gzSize);

            prepare_comp_res(data, gzSize);

#if USE_ALGO_STEP_LOGGING
            algo_change_container(0);
            INFLATE_ALGO_STEP(ALGO_STEP_START_EXEC);
#endif

            gzip_impl = 0;
            int gzr = gzip_decompress(&compRes);

            printf("GZR:ORG: %d | %s\n", gzr, gzip_buffer);

            printf("---\n");
            print_bytes(gzip_tc_result, compRes.result_size, 80, bytes_per_line, 0);
            printf("---\n");

            if (gzr == 0)
            {
                unsigned int print_size = 50;

                if (print_size > compRes.result_size)
                {
                    print_size = compRes.result_size;
                }
                printf("RES:size=%d\n", compRes.result_size);
                printf("RES|%.*s|<END\n", print_size, gzip_tc_result);
            }

#if 1
            prepare_comp_res(data, gzSize);

            fixed_done = 0;
#if USE_ALGO_STEP_LOGGING
            algo_change_container(1);
            INFLATE_ALGO_STEP(ALGO_STEP_START_EXEC);
#endif

            gzip_impl = 2;
            gzr = gzip_decompress(&compRes);

            printf("GZR:AIO: %d | %s\n", gzr, gzip_buffer);

            printf("---\n");
            print_bytes(gzip_tc_result, compRes.result_size, 80, bytes_per_line, 0);
            printf("---\n");

            if (gzr == 0)
            {
                unsigned int print_size = 50;

                if (print_size > compRes.result_size)
                {
                    print_size = compRes.result_size;
                }
                printf("RES:size=%d\n", compRes.result_size);
                printf("RES|%.*s|<END\n", print_size, gzip_tc_result);
            }
#endif
#if 1
            gzip_impl = 1;

            prepare_comp_res(data, gzSize);

            gzr = gzip_decompress(&compRes);
            printf("GZR:TIN: %d |%s|%u\n", gzr, gzip_buffer, inflate_dest_size);

            printf("REAL-IN: %d\n", (int)compRes.real_in_size);
            // printf("---\n");
            // print_bytes(compRes.real_in_data, compRes.real_in_size, 100, 0);
            printf("---\n");
            print_bytes(gzip_tc_result, compRes.result_size, 80, bytes_per_line, 0);
            printf("---\n");
            if (gzr == 0)
            {
                unsigned int print_size = 50;

                if (print_size > compRes.result_size)
                {
                    print_size = compRes.result_size;
                }
                printf("RES:size=%d\n", compRes.result_size);
                printf("RES|%.*s|<END\n", print_size, gzip_tc_result);
            }
#endif
            free(data);

            writeToTestResultFile(tcIdx, 1, b1.data);
            writeToTestResultFile(tcIdx, 2, b2.data);

#if USE_ALGO_STEP_LOGGING
            initTestStringBuffer(&tb_steps, 2 * 1000 * 1000);

            algo_print_steps(0, 1, &tb_steps);
            writeToTestResultFile(100 + tcIdx, 1, tb_steps.data);

            initTestStringBuffer(&tb_steps, 2 * 1000 * 1000);

            algo_print_steps(0, 0, &tb_steps);
            writeToTestResultFile(100 + tcIdx, 11, tb_steps.data);

            initTestStringBuffer(&tb_steps, 2 * 1000 * 1000);

            algo_print_steps(1, 1, &tb_steps);
            writeToTestResultFile(100 + tcIdx, 12, tb_steps.data);
#endif
            printf("END:%d\n\n", (int)tcIdx);
        }
        else
        {
            printf("\nERR:%d|Failed to read file '%s'\n", (int)tcIdx, pInfile);
        }
    }
    freeTestStringBuffer(&b1);
    freeTestStringBuffer(&b2);
    freeTestStringBuffer(&tb_steps);
}

void test_inflate()
{
    test_gzip();
    return;

    size_t gzSize = 0;
    void *data = readFile("ddd.txt.gz", &gzSize);

    if (data != 0)
    {
        printf("GZ:%d\n", (int)gzSize);

        memset(&compRes, 0, sizeof(compRes));
        memset(gz_result, 0, 200);
        compRes.in_data = data;
        compRes.in_size = gzSize;
        compRes.out_data = gz_result;
        compRes.out_size = 200;

        memset(gzip_buffer, 0, sizeof(gzip_buffer));
        gzip_impl = 0;
        int gzr = gzip_decompress(&compRes);
        printf("GZR: %d | %s\n", gzr, gzip_buffer);

        printf("REAL-IN: %d\n", (int)compRes.real_in_size);
        printf("---\n");
        print_bytes(compRes.real_in_data, compRes.real_in_size, 100, 32, 0);
        printf("---\n");
        print_bytes(gz_result, compRes.result_size, 100, 32, 0);
        printf("---\n");

        gzip_impl = 1;
        memset(gzip_buffer, 0, sizeof(gzip_buffer));
        memset(gz_result, 0, sizeof(gz_result));
        gzr = gzip_decompress(&compRes);
        printf("GZR: %d |%s|%u\n", gzr, gzip_buffer, inflate_dest_size);
        printf("REAL-IN: %d\n", (int)compRes.real_in_size);
        printf("---\n");
        print_bytes(compRes.real_in_data, compRes.real_in_size, 100, 32, 0);
        printf("---\n");
        print_bytes(gz_result, compRes.result_size, 100, 32, 0);
        printf("---\n");
        if (gzr == 0)
        {
            printf("RES|%.*s|\n", inflate_dest_size, gz_result);
        }
        gzip_impl = 0;

        free(data);
        return;
    }

    int size = 0;

    int r = inflate(no_compression_result, 4, no_compression, sizeof(no_compression));
    printf("INRES:%d\n", r);
    if (r == 0)
    {
        print_bytes(no_compression_result, 4, 100, 32, 0);
    }

    memset(dec, 0, 128);

    unsigned char *res = (unsigned char *)base64_decode(b, &size);
    printf("Binsize:%d\n", size);
    unsigned int dest_len = 0, src_len = 0;
    if (res != 0)
    {
        memset(dec, 0, 128);
        // printf("%s\n", (char*)res);
        printf("789cf348cdc9c95728c948050010710356\n");
        print_bytes(res, size, 100, 32, 0);

        dest_len = 128;
        src_len = size;
        int tr = tinf_uncompress(dec, &dest_len, res, src_len);
        printf("TINRES:%d | src=%d | dst=%d\n", tr, src_len, dest_len);
        memset(dec, 0, 128);

        print_bytes(res, size, 100, 32, 1);
        int r = inflate(dec, 128, res, size);
        printf("INRES:%d\n", r);

        // printf("\n|%.*s|\n", 20,(char *)dec);

        free(res);
    }
}