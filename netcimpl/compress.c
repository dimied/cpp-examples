
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "file_util.h"
#include "compression/bench_inflate.h"
#include "compression/inflate_test.h"
#include "compression/gzip.h"

/**
 * @param out_data
 * @param out_size
 * @param in_data
 * @param in_size
 * @param buffer with minimal size of 1600 bytes
 */
extern uint64_t inflate_asm_x64(unsigned char *out_data, unsigned int out_size, unsigned char *in_data, unsigned int in_size, void *buffer);
extern int inflate_asm_x64_diffs(unsigned int *sizes);

extern int inflate_asm_x64_clear(void *values, unsigned int len);
unsigned char dst_data[200];
char src_data[150];

uint64_t test_data64[16];

union
{
    uint64_t ui64[260];
    unsigned char ui8[260 * 8];
} asm_buffer;

unsigned int mem_sizes[64];

char *mem_sizes_names[64] = {
    "FUNC_LEN",
    "GET_BITS_LEN",
    "BUFFER_LEN",
    "BUFFER_LEN_8",
    "BUILD_TREE_LEN",
    "DYN_BUILD_TREES_SIZE",
    "BD_GV",
    "NUM_TO_ZERO",
    "NUM_TO_ZERO_8",
    "LAST_TO_ZERO",
    "MEMCPY",
    "STORE_XMM",
    "RESTORE_XMM",
    "RAW_MODE_SIZE",
    "dm_btl_1",
    "dm_btl_2",
    "dm_btl_3",
    "get_bits_error",
    "block_data",
    0};

void print_asm_sizes()
{
    for (int i = 0; i < 24; i++)
    {
        unsigned int ms = mem_sizes[i];
        printf("S:%2d : %20s : %8d | %x\n", i, mem_sizes_names[i], ms, ms);
    }
}

void test_clear()
{
    memset(test_data64, 0, sizeof(test_data64));
    int i;
    for (i = 0; i < 16; i++)
    {
        test_data64[i] = (i + 1) * (i + 1);
        printf("I:%2d: %ld\n", i, test_data64[i]);
    }

    inflate_asm_x64_clear(&test_data64[7], 8);

    for (i = 0; i < 16; i++)
    {
        printf("O:%2d: %ld\n", i, test_data64[i]);
    }
}

unsigned long somedata[128];

unsigned char test1_data[32];
unsigned char test1_data_expected[32];

extern unsigned int inflate_test1(unsigned char *pMem, uint64_t mask, unsigned short);

static const unsigned char test_clcidx[15] = {
    8, 7, 9, 6,
    10, 5, 11, 4,
    12, 3, 13, 2,
    14, 1, 15};

void init_random()
{
    // Use a different seed value so that we don't get same
    // result each time we run this program
    srand(time(NULL));
}
void randomize(unsigned char *arr, int n)
{
    // Start from the last element and swap one by one.
    // We don't need to run for the first element
    // that's why i > 0
    for (int i = n - 1; i > 0; i--)
    {
        // Pick a random index from 0 to i
        int j = rand() % (i + 1);

        unsigned char t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
    }
}

void test1()
{
    const size_t test_data_size = sizeof(test1_data);

    init_random();
    printf("T1\n");
    size_t i;

    size_t test_run, max_test_runs = 1000*1000;

    for (test_run = 0; test_run < max_test_runs; test_run++)
    {
        memset(test1_data, 0, sizeof(test1_data));
        memset(test1_data_expected, 0, sizeof(test1_data_expected));

        unsigned int test_size = 15;

        uint64_t test_bits = 0;
        for (i = 0; i < test_size; i++)
        {
            unsigned char v = rand() % 8;
            int idx = test_clcidx[i];
            test1_data_expected[idx] = v;
            test_bits |= ((uint64_t)v) << (i * 3);
        }
        int res = inflate_test1(test1_data, test_bits, test_size);

        int test_cmp_res = memcmp(test1_data, test1_data_expected, test_data_size);

        if (res < 0 || (test_cmp_res != 0))
        {
            printf("TR: %ld\n", test_run);
            printf("F-RES: %d\n", res);
            printf("C-RES: %d\n", test_cmp_res);
            for (i = 0; i < 16; i++)
            {
                printf("%d ", test1_data[i]);
            }
            printf("\n");
            for (i = 0; i < 16; i++)
            {
                printf("%d ", test1_data_expected[i]);
            }
            printf("\n");
            break;
        }
    }

    memset(test1_data, 0, sizeof(test1_data));
    uint64_t mask = 0x3 | (0x5 << 3) | (2 << 6) | (7 << 9) | (1 << 12) | (6 << 15);

    printf("\n");

    inflate_test1(test1_data, mask, 15);

    for (i = 0; i < 16; i++)
    {
        printf("%d ", test1_data[i]);
    }
    printf("\n");

    int cmpRes = memcmp(test1_data, test1_data_expected, test_data_size);
    printf("CMP-RES: %d\n\n", cmpRes);
}

int main()
{
    // uint64_t sv = ((6UL) | (256UL << 6) | (36UL << 16) | (0) | (2UL << 32) | (280UL << 38) | (28UL << 48) | (144UL << 54));
    // printf("V: %lu\n", sv);

    test1();

    // test_inflate();
    // test_clear();
    memset(mem_sizes, 0, sizeof(mem_sizes));
    memset(src_data, 0, 100);
    memset(dst_data, 0, 100);

#if 1

#if 1
    src_data[0] = 3;
    src_data[1] = 2;
    src_data[2] = 1;
    dst_data[0] = 1;
    dst_data[1] = 2;
    dst_data[2] = 3;
#endif
    // inflate_asm_x64_addr(&pAddr);

    inflate_asm_x64_diffs(mem_sizes);
    printf("DIFF...\n");

    uint64_t ar = inflate_asm_x64(dst_data, 200, (unsigned char *)src_data, 150, asm_buffer.ui64);
    printf("RES:%lx | %ld\n", ar, ar);
    printf("RES:%p | %p | %p\n", (void *)ar, src_data, dst_data);
#if 0
    for (int i = 0; i < 8; i++)
    {
        printf("@%d: %d\n", i, (int)src_data[i]);
    }
    for (int i = 0; i < 8; i++)
    {
        printf("@%d: %d\n", i, dst_data[i]);
    }
#endif
    print_asm_sizes();
#endif

    return 0;
}
