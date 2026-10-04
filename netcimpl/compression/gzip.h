#ifndef COMPRESSION_GZIP_INCLUDED
#define COMPRESSION_GZIP_INCLUDED

#include <stdint.h>

#ifndef ADD_TO_SB
#define ADD_TO_SB(B, FORMAT, ...)                                 \
    do                                                            \
    {                                                             \
        char temp_buffer[256];                                     \
        memset(temp_buffer, 0, sizeof(temp_buffer));              \
        int temp_len = sprintf(temp_buffer, FORMAT, __VA_ARGS__); \
        addToTestStringBuffer(&B, temp_buffer, temp_len);         \
    } while (0)
#endif

#include "compression_types.h"
#include "inflate.h"

#define GZIP_DECOMPRESS_OK 0
#define GZIP_DECOMPRESS_ERROR_TOO_SMALL 1
#define GZIP_DECOMPRESS_ERROR_WRONG_START 2
#define GZIP_DECOMPRESS_ERROR_FILENAME 3
#define GZIP_DECOMPRESS_ERROR_NOMEM 4
#define GZIP_DECOMPRESS_ERROR_INVALID_ARG 10

#define FLAG_FHRC 0x2
#define FLAG_FEXTRA 0x4
#define FLAG_FNAME 0x8
#define FLAG_FCOMMENT 0x10

#define GZIP_PATH_MAX 256

char gzip_buffer[GZIP_PATH_MAX];

int gzip_impl = 0;
unsigned int inflate_dest_size = 0;
unsigned int inflate_res = 0;

int gzip_decompress(CompressionResult *pResult)
{
    // Inspired by https://github.com/byuu/higan/blob/master/nall/decode/gzip.hpp

    if (pResult == 0)
    {
        return GZIP_DECOMPRESS_ERROR_INVALID_ARG;
    }

    unsigned char *data = pResult->in_data;
    unsigned int size = pResult->in_size;

    if (size < 18)
    {
        return GZIP_DECOMPRESS_ERROR_TOO_SMALL;
    }
    uint64_t *pHeader = (uint64_t *)data;
    uint64_t header1 = *pHeader;
    ++pHeader;

    if ((header1 & 0xFFFF) != (uint64_t)(0x8b1f))
    {
        return GZIP_DECOMPRESS_ERROR_WRONG_START;
    }
    header1 >>= 16;

#if 0
    unsigned int cm = (unsigned int)(header1 & 0xFF);
#endif
    header1 >>= 8;
    unsigned int flg = (unsigned int)(header1 & 0xFF);
    header1 >>= 8;

    // bytes: 4,5,6,7
    unsigned int mtime = (unsigned int)header1;

    header1 = *pHeader;

    // 8
#if 0    
    unsigned int xfl = (unsigned int)(header1 & 0xFF);
#endif
    // 9
#if 0
    //header1 >>= 8;
    //unsigned int os = (unsigned int)(header1 & 0xFF);
#endif
    unsigned int p = 10;

    unsigned int isize = 0;
    // last 4 bytes for the size
    {
        uint64_t addr = (uint64_t)data + (size - 4);
        // aligned?
        if ((addr & 0x3) == 0)
        {
            isize = *(unsigned int *)addr;
        }
        else
        {
            isize = data[size - 4];
            isize |= data[size - 3] << 8;
            isize |= data[size - 2] << 16;
            isize |= data[size - 1] << 24;
        }
    }

    pResult->mtime = mtime;

    if ((flg & FLAG_FEXTRA) != 0)
    { // FEXTRA
        uint xlen = data[p + 0];
        xlen |= data[p + 1] << 8;
        p += 2 + xlen;
    }

    if ((flg & FLAG_FNAME) != 0)
    { // FNAME

        for (uint n = 0; n < GZIP_PATH_MAX; n++, p++)
        {
            gzip_buffer[n] = data[p];
            if (data[p] == 0)
            {
                break;
            }
        }
        if (data[p++] != 0)
        {
            return GZIP_DECOMPRESS_ERROR_FILENAME;
        }
    }

    if ((flg & FLAG_FCOMMENT) != 0)
    {
        // Just skip...
        while (data[p++] != 0)
        {
        }
    }

    if ((flg & FLAG_FHRC) != 0)
    {
        p += 2;
    }

    if (pResult->out_size == 0)
    {
        if (pResult->out_data != 0)
        {
            return -2000;
        }
    }

    if (pResult->out_data == 0)
    {
        return GZIP_DECOMPRESS_ERROR_NOMEM;
    }
    if (isize > pResult->out_size)
    {
        return -222;
    }
    unsigned char *result = (unsigned char *)pResult->out_data;

    pResult->result_size = isize;
    pResult->real_in_data = data + p;
    pResult->real_in_size = size - p - 8;

    if (gzip_impl == 1)
    {
        inflate_dest_size = isize;
        return tinf_uncompress(result, &inflate_dest_size, pResult->real_in_data, pResult->real_in_size);
    }
    else if (gzip_impl == 2)
    {
        return inflate_aio(result, isize, data + p, size - p - 8);
    }

    return inflate(result, isize, data + p, size - p - 8);
}

#endif