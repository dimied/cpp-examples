#ifndef TEST_STRING_BUFFER
#define TEST_STRING_BUFFER

#include <string.h>

typedef struct TestStringBuffer
{
    size_t size;
    size_t used;
    char verbose;
    /*@null@*/ char *data;
} TestStringBuffer;

void clearTestStringBuffer(/*@null@*/ TestStringBuffer *pBuffer)
{
    if (pBuffer != 0)
    {
        memset(pBuffer, 0, sizeof(TestStringBuffer));
    }
}

void freeTestStringBuffer(/*@null@*/ TestStringBuffer *pBuffer)
{
    if (pBuffer != 0 && pBuffer->data != 0)
    {
        free(pBuffer->data);
        pBuffer->data = (char *)0;
    }
}

int addToTestStringBuffer(/*@null@*/ TestStringBuffer *pBuffer, /*@null@*/ char *data, size_t len)
{
    if (pBuffer != 0)
    {
        if (len != 0)
        {
            if (data == 0)
            {
                return 3;
            }
            if (pBuffer->size != 0)
            {
                if (pBuffer->data == 0)
                {
                    return 2;
                }

                long int mem_diff = 0;
                if (pBuffer->data > data)
                {
                    mem_diff = (long int)(pBuffer->data - data);
                }
                else
                {
                    mem_diff = (long int)(data - pBuffer->data);
                }
                if (mem_diff == 0)
                {
                    return 0;
                }
                // TODO: overlapping memory

                if (pBuffer->used + len <= pBuffer->size)
                {
                    memcpy(pBuffer->data + pBuffer->used, data, len);
                }
                else
                {
                    return 3;
                }
            }
            // otherwise just count

            pBuffer->used += len;
        }
        return 0;
    }
    return 1;
}

int initTestStringBuffer(TestStringBuffer *pBuffer, size_t len)
{
    if (pBuffer != 0)
    {
        if (len != 0)
        {
            if (pBuffer->data != 0)
            {
                free(pBuffer->data);
            }
            pBuffer->data = 0;
            pBuffer->size = 0;
            pBuffer->used = 0;

            void *data = calloc(len, 1);

            if (data != 0)
            {
                if (pBuffer->verbose != (char)0)
                {
                    printf("//MEM-ALLOCATED: %d\n", (int)len);
                }

                pBuffer->size = len;
                pBuffer->data = (char *)data;
                return 0;
            }

            return 1;
        }
        return 0;
    }
    return 1;
}

#endif