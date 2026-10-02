
#include "base64.h"

#include "tinf.h"

#include "inflate.h"
#include "gzip.h"

#if 0
typedef struct Inflate_TC
{
    char *in;
    size_t a;
} Inflate_TC;
#endif

void print_bytes(unsigned char *bytes, size_t num, size_t max, int sep)
{
    if (num > max)
    {
        num = max;
    }
    size_t i;
    for (i = 0; i < num; i++)
    {
        if (i != 0 && i % 32 == 0)
        {
            printf("\n");
        }
        if (sep)
        {
            printf(" ");
        }
        printf("%02x", bytes[i]);
    }
    printf("\n");
}

int inflate(unsigned char *out_data, unsigned int out_size, unsigned char *in_data, unsigned int in_size);

unsigned char dec[128];

unsigned char no_compression[] = {0x4, 0x4, 0x0, 0xFB, 0xFF, 0x21, 0x56, 0x76, 0xF2};
unsigned char no_compression_result[4];

char *b = "eJzzSM3JyVcoyUgFABBxA1Y=";

CompressionResult compRes;

unsigned char gz_result[200];

void test_inflate()
{
    {
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
            int gzr = gzip_decompress(&compRes);
            printf("GZR: %d | %s\n", gzr, gzip_buffer);

            printf("REAL-IN: %d\n", (int)compRes.real_in_size);
            printf("---\n");
            print_bytes(compRes.real_in_data, compRes.real_in_size, 100, 0);
            printf("---\n");
            print_bytes(gz_result, compRes.result_size, 100, 0);
            printf("---\n");

            gzip_impl = 1;
            memset(gzip_buffer, 0, sizeof(gzip_buffer));
            memset(gz_result, 0, sizeof(gz_result));
            gzr = gzip_decompress(&compRes);
            printf("GZR: %d |%s|%u\n", gzr, gzip_buffer, inflate_dest_size);
            printf("REAL-IN: %d\n", (int)compRes.real_in_size);
            printf("---\n");
            print_bytes(compRes.real_in_data, compRes.real_in_size, 100, 0);
            printf("---\n");
            print_bytes(gz_result, compRes.result_size, 100, 0);
            printf("---\n");
            if (gzr == 0)
            {
                printf("RES|%.*s|\n", inflate_dest_size, gz_result);
            }
            gzip_impl = 0;

            free(data);
            return;
        }
    }

    int size = 0;

    int r = inflate(no_compression_result, 4, no_compression, sizeof(no_compression));
    printf("INRES:%d\n", r);
    if (r == 0)
    {
        print_bytes(no_compression_result, 4, 100, 0);
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
        print_bytes(res, size, 100, 0);

        dest_len = 128;
        src_len = size;
        int tr = tinf_uncompress(dec, &dest_len, res, src_len);
        printf("TINRES:%d | src=%d | dst=%d\n", tr, src_len, dest_len);
        memset(dec, 0, 128);

        print_bytes(res, size, 100, 1);
        int r = inflate(dec, 128, res, size);
        printf("INRES:%d\n", r);

        // printf("\n|%.*s|\n", 20,(char *)dec);

        free(res);
    }
}