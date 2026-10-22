
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
    uint64_t ui64[25];
    unsigned char ui8[1280];
} asm_buffer;

unsigned int mem_sizes[32];
char *mem_sizes_names[32] = {
    "FUNC_LEN",
    "GET_BITS_LEN",
    "NUM_TO_ZERO",
    "NUM_TO_ZERO_8",
    "LAST_TO_ZERO",
    "RAW_JUMP",
    "FIXED_JUMP",
    "DYN_JUMP",
    "MODE_ADDRS",
    0
};

void print_asm_sizes()
{
    for (int i = 0; i < 16; i++)
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

extern void inflate_asm_x64_fixed_addr(void* p);

void *pAddr = 0;

int main() {
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
    inflate_asm_x64_fixed_addr(&pAddr);
    printf("ADDR: %p | %p\n", pAddr, (void *)&inflate_fixed_pc.both);

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
