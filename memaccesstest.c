

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct _TypedArray
{
    void *mem;
    unsigned int typeAndOptions;
    unsigned int used;
    unsigned int size;
} TypedArray2;

#define MANAGED_MEM_IMPL 1

typedef TypedArray2 ManagedMemTypedArray;

ManagedMemTypedArray* createManagedMem();
int freeManagedMem(ManagedMemTypedArray* pMem);

#include "managedmem.h"

int main(int argc, char **argv)
{
    int res = initMem();
    printf("INIT: %d\n", res);
    res = deinitMem();
    printf("DE-INIT: %d\n", res);
    return 0;
}


int freeManagedMem(ManagedMemTypedArray* pMem) {
    if (pMem != 0) {
        free(pMem->mem);
        return 0;
    }
    return 1;
}