#ifndef TEST_STRING_BUFFER
#define TEST_STRING_BUFFER

#include <string.h>

typedef struct TestStringBuffer
{
    size_t size;
    size_t used;
    char verbose;
    char *data;
} TestStringBuffer;

void clearTestStringBuffer(TestStringBuffer *pBuffer)
{
    memset(pBuffer, 0, sizeof(TestStringBuffer));
}

int addToTestStringBuffer(TestStringBuffer *pBuffer, char *data, size_t len)
{
    if (pBuffer != 0)
    {
        if (len != 0)
        {
            if (pBuffer->size != 0)
            {
                if (pBuffer->data == 0)
                {
                    return 2;
                }

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
                if (pBuffer->verbose)
                {
                    printf("//MEM-ALLOCATED: %ld\n", len);
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