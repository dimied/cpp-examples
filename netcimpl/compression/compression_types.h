#ifndef COMPRESSION_TYPES
#define COMPRESSION_TYPES

typedef struct CompressionResult
{
    int type;
    unsigned int in_size;
    unsigned int real_in_size;
    unsigned int out_size;
    unsigned int result_size;
    unsigned int mtime;

    unsigned char *in_data;
    unsigned char *real_in_data;
    unsigned char *out_data;
} CompressionResult;

#endif