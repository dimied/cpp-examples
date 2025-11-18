#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include "b64.h"

extern void naivebase64(unsigned char *pSrc, int length, unsigned char *pDst);

int calcOutputSize(int n)
{
    return (n + 2) / 3 * 4;
}

unsigned char *genRandom(int size)
{
    unsigned char *pResult = (unsigned char *)malloc(size);
    if (pResult != 0)
    {
        for (int i = 0; i < size; i++)
        {
            pResult[i] = (unsigned char)rand() % 256;
        }
    }

    return pResult;
}

using namespace std::chrono;

void performTest(int size)
{
    int outputSize = calcOutputSize(size);
    printf("Started for %i -> %i\n", size, outputSize);

    unsigned char *pRandomData = genRandom(size);

    if (pRandomData == 0)
    {
        printf("Failed to generate random data\n");
        return;
    }
    unsigned char *pOutput = (unsigned char *)malloc(outputSize);
    if (pOutput == 0)
    {
        printf("Failed to allocate output buffer\n");
        free(pRandomData);
        return;
    }
     unsigned char *pOutput2 = (unsigned char *)malloc(outputSize);
    if (pOutput2 == 0)
    {
        printf("Failed to allocate output buffer\n");
        free(pRandomData);
        free(pOutput);
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();

    naivebase64(pRandomData, size, pOutput);

    auto end = std::chrono::high_resolution_clock::now();

    auto time_span = duration_cast<microseconds>(end - start);

    printf("Naive: %.4lf ms\n", time_span.count()/1000.0f);

    start = std::chrono::high_resolution_clock::now();

    to64(pRandomData, size, pOutput2);

    end = std::chrono::high_resolution_clock::now();

    time_span = duration_cast<microseconds>(end - start);

    printf("No lookup: %.4lf ms\n", time_span.count()/1000.0f);

    free(pRandomData);
    free(pOutput);
    free(pOutput2);
}

int main()
{
    performTest(1024*1024);

    return 0;
}