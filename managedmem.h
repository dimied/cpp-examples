#ifndef MANAGED_MEM_INCLUDED
#define MANAGED_MEM_INCLUDED

#endif

#ifdef MANAGED_MEM_DECLARATIONS
int initMem();
int deinitMem();

#endif

#ifndef MANAGED_MEM_CONSTANTS
#define MANAGED_MEM_CONSTANTS

#define MEM_SUCCESS (0)
#define MEM_ERROR (1)
#define MEM_ERROR_INVALID_TYPE_OFFSET (2)
#define MEM_ERROR_INVALID_TYPE_PTR (3)
#define MEM_ERROR_INVALID_ITEM_OFFSET (4)

#endif

#ifdef MANAGED_MEM_IMPL
#if MANAGED_MEM_IMPL

#define MANAGED_MEM_IS_NULL_PTR(ptr) (ptr == 0)
#define MEM_OFFSET_LIMIT (0x1 << 31)

void *redirectMem = (void *)0;
ManagedMemTypedArray **types = (void *)0;
const int redirectMemEntries = 10000;
int typedArraySize = 100;


#if 0
void managedMemResetMem(void *p, size_t sz)
{
    if (p != 0)
    {
        memset(p, 0, sz);
    }
}
#else
#define managedMemResetMem(PTR, SZ)       \
    do                          \
    {                           \
        if (PTR != 0)           \
        {                       \
            memset(PTR, 0, SZ); \
        }                       \
    } while (0)
#endif

void *getRealObject(void *p, int *error)
{
    if (redirectMem != 0 && redirectMem <= p && (p - redirectMem) < redirectMemEntries)
    {
        unsigned int value = *((unsigned int *)p);
        // 0 means not used
        if (value >= 0x10000)
        {
            unsigned int typeOffset = (value >> 16);
            int err = MEM_ERROR_INVALID_TYPE_OFFSET;

            if (typeOffset < typedArraySize)
            {
                ManagedMemTypedArray *tArr = types[typeOffset];
                if (tArr != 0)
                {
                    unsigned int offset = (value & 0xFFFF);
                    if (offset < tArr->size)
                    {
                        // TODO: alignment options
                        unsigned long int typeSize = tArr->typeAndOptions & 0xFFFF;
                        typeSize *= offset;

                        if (typeSize < MEM_OFFSET_LIMIT)
                        {
                            tArr++;
                            unsigned char *pBytePtr = (unsigned char *)tArr;
                            return (void *)(pBytePtr + typeSize * offset);
                        }
                    }

                    err = MEM_ERROR_INVALID_ITEM_OFFSET;
                }
                else
                {
                    err = MEM_ERROR_INVALID_TYPE_PTR;
                }
            }
            if (error != 0)
            {
                *error = err;
            }
            return 0;
        }
    }
    return p;
}

int initMem()
{
    if (MANAGED_MEM_IS_NULL_PTR(redirectMem))
    {
        size_t sz = redirectMemEntries * sizeof(unsigned int);
        void *mem = malloc(sz);
        if (mem != 0)
        {
            redirectMem = mem;
            //
            managedMemResetMem(redirectMem, sz);
            //
            if (MANAGED_MEM_IS_NULL_PTR(types))
            {
                sz = typedArraySize * sizeof(ManagedMemTypedArray *);
                mem = malloc(sz);

                if (mem != 0)
                {
                    types = mem;
                    //
                    managedMemResetMem(types, sz);
                    //
                    return MEM_SUCCESS;
                }
                free(redirectMem);
                redirectMem = (void *)0;
            }
            else
            {
                return MEM_SUCCESS;
            }
        }
    }
    return MEM_ERROR;
}

void releaseTypedArrays()
{
    for (int i = 0; i < typedArraySize; i++)
    {
        ManagedMemTypedArray *pArr = types[i];

        if (pArr != 0)
        {
            freeManagedMem(pArr);
            types[i] = 0;
        }
    }
}

int deinitMem()
{
    if (redirectMem != 0)
    {
        if (types != 0)
        {
            releaseTypedArrays();
            free(types);
            types = 0;
        }
        free(redirectMem);
        redirectMem = 0;

        return MEM_SUCCESS;
    }
    return MEM_ERROR;
}

#endif
#endif