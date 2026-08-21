#ifndef GEN_CODE_IN_C
#define GEN_CODE_IN_C

#include <string.h>

#define CODEGEN_TYPE_TYPE_GENERIC 1
#define CODEGEN_TYPE_TYPE_DECL 2
#define CODEGEN_TYPE_ARRAY_TYPE_DECL 3
#define CODEGEN_TYPE_STRUCT 4

#define ERROR_CODE_INVALID_ARGUMENTS 1
#define ERROR_CODE_FAILED_CREATION 2
#define ERROR_CODE_FAILED_COPY 3

#define CODEGEN_OPTION_INLINE_NAME (0x1)
#define CODEGEN_OPTION_INLINE_TYPE (0x2)
#define CODEGEN_OPTION_INLINE_ARR_VALUE (0x4)

#define SET_OPTION_INLINE_NAME(OPTS) (OPTS |= CODEGEN_OPTION_INLINE_NAME)
#define UNSET_OPTION_INLINE_NAME(OPTS) (OPTS &= (~CODEGEN_OPTION_INLINE_NAME))
#define HAS_OPTION_INLINE_NAME(OPTS) ((OPTS & CODEGEN_OPTION_INLINE_NAME) != 0)
#define HAS_NO_OPTION_INLINE_NAME(OPTS) ((OPTS & CODEGEN_OPTION_INLINE_NAME) == 0)

#define SET_OPTION_INLINE_TYPE(OPTS) (OPTS |= CODEGEN_OPTION_INLINE_TYPE)
#define UNSET_OPTION_INLINE_TYPE(OPTS) (OPTS &= (~CODEGEN_OPTION_INLINE_TYPE))
#define HAS_OPTION_INLINE_TYPE(OPTS) ((OPTS &= CODEGEN_OPTION_INLINE_TYPE) != 0)
#define HAS_NO_OPTION_INLINE_TYPE(OPTS) ((OPTS & CODEGEN_OPTION_INLINE_TYPE) == 0)

#define SET_OPTION_INLINE_ARR_VALUE(OPTS) (OPTS |= CODEGEN_OPTION_INLINE_ARR_VALUE)
#define UNSET_OPTION_INLINE_ARR_VALUE(OPTS) (OPTS &= (~CODEGEN_OPTION_INLINE_ARR_VALUE))
#define HAS_OPTION_INLINE_ARR_VALUE(OPTS) ((OPTS &= CODEGEN_OPTION_INLINE_ARR_VALUE) != 0)
#define HAS_NO_OPTION_INLINE_ARR_VALUE(OPTS) ((OPTS & CODEGEN_OPTION_INLINE_ARR_VALUE) == 0)

#define SET_OPTION_SINGLE_SUB(OPTS) (OPTS |= 0x8)

/**
 * Linked list storing pointers
 */
struct codegen_pointer_container_type
{
    // void *up;
    unsigned int type;
    struct codegen_pointer_container_type *next;
    void *content[6];
};

typedef union
{
    char *pName;
    char name[sizeof(char *)];
} CharArrayOrPointer;

struct codegen_container_type
{
    unsigned int type;
    unsigned int options;
    unsigned int refCount;
    size_t arrCount;
    CharArrayOrPointer name;
    CharArrayOrPointer typeName;
    CharArrayOrPointer arrInitValue;
    struct codegen_container_type *next;
    struct codegen_container_type *sub;
    // TODO // or union for to store pointer to single value
    // struct codegen_pointer_container_type *subs;
};

typedef struct codegen_container_type codegen_container;

#define MAX_NUM_ALLOCATIONS (100 * 1024)
void *genCodeMemoryPointers[MAX_NUM_ALLOCATIONS];
unsigned char genCodeTypes[MAX_NUM_ALLOCATIONS];
unsigned char genCodeSources[MAX_NUM_ALLOCATIONS];
unsigned char genCodeOptions[MAX_NUM_ALLOCATIONS];
size_t genCodeSizes[MAX_NUM_ALLOCATIONS];

int initMemCalled = 0;
int numMemAllocItems = 0;

enum AllocationType
{
    UNKNOWN_TYPE = 0,
    STRING_TYPE,
    CONTAINER_TYPE,
    INVALID_TYPE
};

enum AllocationSource
{
    SOURCE_CONTAINER,
    SOURCE_STRUCT,
    SOURCE_STRUCT_ADD,
    SOURCE_OUTPUT
};

void initMem()
{
    if (initMemCalled == 0)
    {
        memset(genCodeMemoryPointers, 0, sizeof(genCodeMemoryPointers));
        memset(genCodeTypes, 0, sizeof(genCodeTypes));
        memset(genCodeSizes, 0, sizeof(genCodeSizes));
        memset(genCodeSources, 0, sizeof(genCodeSources));
        memset(genCodeOptions, 0, sizeof(genCodeOptions));
    }
    ++initMemCalled;
}

void *allocMemoryImpl(size_t size, unsigned char type, unsigned char source, int *pItemNum)
{
    if (numMemAllocItems < MAX_NUM_ALLOCATIONS)
    {
        void *result = malloc(size);
        if (result != 0)
        {
            genCodeMemoryPointers[numMemAllocItems] = result;
            genCodeSizes[numMemAllocItems] = size;
            genCodeTypes[numMemAllocItems] = type;

            if (pItemNum != 0)
            {
                *pItemNum = numMemAllocItems;
            }
            ++numMemAllocItems;
        }
        return result;
    }
    return 0;
}

void *allocMemory(size_t size, unsigned char type, unsigned char source)
{
    if (type > 0x7F)
    {
        printf("Invalid type alloc!\n");
    }
    return allocMemoryImpl(size, type, source, NULL);
}

void *callocMemory(size_t size, unsigned char type, unsigned char source)
{
    // mark type as zero initialized
    void *result = allocMemoryImpl(size, type | 0x80, source, NULL);
    if (result != 0)
    {
        memset(result, 0, size);
    }
    return result;
}

void freeMemory(void *pMem)
{
    if (pMem != 0)
    {
        for (size_t i = 0; i < numMemAllocItems; i++)
        {
            if (pMem == genCodeMemoryPointers[i])
            {
                if ((genCodeOptions[i] & 0x1) == 0)
                {
                    free(pMem);
                    genCodeOptions[i] |= 1;
                    // genCodeMemoryPointers[i] = (void *)0;
                }
                else
                {
                    printf("Trying to deallocate twice @%ld | %p\n", i, pMem);
                }

                return;
            }
        }
        printf("Failed to find memory");
    }
}

/**
 * @param pRes
 * @param code
 */
void codeGenSetResult(int *pRes, int code)
{
    if (pRes != 0)
    {
        *pRes = code;
    }
}

/**
 * @param len
 */
void *mallocZeroInit(size_t len)
{
    return calloc(len, 1);
}

/**
 *
 */
codegen_container *codeGenCreateContainer()
{
    return (codegen_container *)mallocZeroInit(sizeof(codegen_container));
}

struct PropertyAssignmentStats
{
    size_t forName;
    size_t forNameCopy;
    size_t forType;
    size_t forTypeCopy;
    size_t forArrValue;
    size_t forArrValueCopy;
} assignmentStats = {0, 0, 0};

int printAssignmentStats(char *pDst, size_t dstLen, struct PropertyAssignmentStats *pSrc)
{
    if (pSrc == 0)
    {
        pSrc = &assignmentStats;
    }

    size_t n1 = pSrc->forName, n2 = pSrc->forNameCopy;
    size_t t1 = pSrc->forType, t2 = pSrc->forTypeCopy;
    size_t a1 = pSrc->forArrValue, a2 = pSrc->forArrValueCopy;
    size_t nSum = n1 + n2, tSum = t1 + t2, aSum = a1 + a2;

    if (pDst != 0)
    {
        size_t s = snprintf(pDst, dstLen, "Stats(direct/copy/sum): #name: %ld/%ld/%ld | type: %ld/%ld/%ld | arrValue: %ld/%ld/%ld\n",
                            n1, n2, nSum, t1, t2, tSum, a1, a2, aSum);
        if (s < dstLen)
        {
            return 0;
        }
        return 1;
    }
    printf("Stats(direct/copy/sum): #name: %ld/%ld/%ld | type: %ld/%ld/%ld | arrValue: %ld/%ld/%ld\n",
           n1, n2, nSum, t1, t2, tSum, a1, a2, aSum);
    return 0;
}

char *getNamePtr(codegen_container *pContainer)
{
    if (pContainer == 0)
    {
        return 0;
    }
    return HAS_OPTION_INLINE_NAME(pContainer->options) ? pContainer->name.name : pContainer->name.pName;
}

void *setName(codegen_container *pResult, char *pName, size_t nameLen)
{
    if (nameLen < sizeof(CharArrayOrPointer))
    {
        memset(pResult->name.name, 0, sizeof(CharArrayOrPointer));
        strncpy(pResult->name.name, pName, nameLen);
        SET_OPTION_INLINE_NAME(pResult->options);
        assignmentStats.forName++;
    }
    else
    {
        char *nameCopy = (char *)mallocZeroInit(nameLen + 1);
        if (nameCopy == 0)
        {
            return 0;
        }
        UNSET_OPTION_INLINE_NAME(pResult->options);
        strncpy(nameCopy, pName, nameLen);
        pResult->name.pName = nameCopy;
        assignmentStats.forNameCopy++;
    }
    return pResult;
}

char *getTypeNamePtr(codegen_container *pContainer)
{
    if (pContainer == 0)
    {
        return 0;
    }
    return HAS_OPTION_INLINE_TYPE(pContainer->options) ? pContainer->typeName.name : pContainer->typeName.pName;
}

void *setTypeName(codegen_container *pResult, char *pTypeName, size_t nameLen)
{
    if (nameLen < sizeof(CharArrayOrPointer))
    {
        memset(pResult->typeName.name, 0, sizeof(CharArrayOrPointer));
        strncpy(pResult->typeName.name, pTypeName, nameLen);
        SET_OPTION_INLINE_TYPE(pResult->options);
        assignmentStats.forType++;
    }
    else
    {
        char *nameCopy = (char *)mallocZeroInit(nameLen + 1);
        if (nameCopy == 0)
        {
            return 0;
        }
        UNSET_OPTION_INLINE_NAME(pResult->options);
        strncpy(nameCopy, pTypeName, nameLen);
        pResult->typeName.pName = nameCopy;
        assignmentStats.forTypeCopy++;
    }
    return pResult;
}

char *getArrInitValuePtr(codegen_container *pContainer)
{
    if (pContainer == 0)
    {
        return 0;
    }
    return HAS_OPTION_INLINE_ARR_VALUE(pContainer->options)
               ? pContainer->arrInitValue.name
               : pContainer->arrInitValue.pName;
}

void *setArrInitValue(codegen_container *pResult, char *pArrInitValue, size_t nameLen)
{
    if (nameLen < sizeof(CharArrayOrPointer))
    {
        memset(pResult->arrInitValue.name, 0, sizeof(CharArrayOrPointer));
        strncpy(pResult->arrInitValue.name, pArrInitValue, nameLen);
        SET_OPTION_INLINE_ARR_VALUE(pResult->options);
        assignmentStats.forArrValue++;
    }
    else
    {
        char *nameCopy = (char *)mallocZeroInit(nameLen + 1);
        if (nameCopy == 0)
        {
            return 0;
        }
        UNSET_OPTION_INLINE_ARR_VALUE(pResult->options);
        strncpy(nameCopy, pArrInitValue, nameLen);
        pResult->arrInitValue.pName = nameCopy;
        assignmentStats.forArrValueCopy++;
    }
    return pResult;
}

/**
 * @param pContainer
 */
int codeGenFree(codegen_container *pContainer)
{
    if (pContainer == 0)
    {
        return ERROR_CODE_INVALID_ARGUMENTS;
    }

    // printf("Start:codeGenFree\n");

    if (HAS_NO_OPTION_INLINE_NAME(pContainer->options) && pContainer->name.pName != (char *)0)
    {
        // printf("FREE:name...");
        free(pContainer->name.pName);
        pContainer->name.pName = 0;
        // printf("DONE\n");
    }

    if (HAS_NO_OPTION_INLINE_TYPE(pContainer->options) && pContainer->typeName.pName != (char *)0)
    {
        // printf("FREE:type...");
        free(pContainer->typeName.pName);
        pContainer->typeName.pName = 0;
        // printf("DONE\n");
    }

    if (HAS_NO_OPTION_INLINE_ARR_VALUE(pContainer->options) && pContainer->arrInitValue.pName != (char *)0)
    {
        // printf("FREE:arrInit...");
        free(pContainer->arrInitValue.pName);
        pContainer->arrInitValue.pName = 0;
        // printf("DONE\n");
    }

    if (pContainer->next != 0)
    {
        // printf("FREE:next...\n");
        codeGenFree(pContainer->next);
        pContainer->next = 0;
        // printf("DONE\n");
    }

    // printf("FREE:CONTAINER...\n");
    free(pContainer);

    return 0;
}

codegen_container *codeGenCreateTypedDeclaration(char *type, char *name, int *pRes)
{
    if ((type == 0) || (name == 0))
    {
        codeGenSetResult(pRes, ERROR_CODE_INVALID_ARGUMENTS);
        return 0;
    }

    codegen_container *propRes = codeGenCreateContainer();
    if (propRes == 0)
    {
        codeGenSetResult(pRes, ERROR_CODE_FAILED_CREATION);
        return 0;
    }

    size_t nameLen = strlen(type);
    void *pSetRes = setTypeName(propRes, type, nameLen);
    if (pSetRes == 0)
    {
        codeGenSetResult(pRes, ERROR_CODE_FAILED_COPY);
        codeGenFree(propRes);
        return 0;
    }

    nameLen = strlen(name);
    pSetRes = setName(propRes, name, nameLen);

    if (pSetRes == 0)
    {
        codeGenSetResult(pRes, ERROR_CODE_FAILED_COPY);
        codeGenFree(propRes);
        return 0;
    }

    propRes->type = CODEGEN_TYPE_TYPE_DECL;

    return propRes;
}

codegen_container *codeGenCreateArrayDeclaration(char *type, char *name, size_t size, char *arrInitValue, int *pRes)
{
    codegen_container *propRes = codeGenCreateTypedDeclaration(type, name, pRes);
    if (propRes == 0)
    {
        return propRes;
    }

    size_t nameLen = strlen(arrInitValue);
    void *pSetRes = setArrInitValue(propRes, arrInitValue, nameLen);
    if (pSetRes == 0)
    {
        codeGenFree(propRes);
        return 0;
    }

    propRes->type = CODEGEN_TYPE_ARRAY_TYPE_DECL;
    propRes->arrCount = size;

    return propRes;
}

/**
 * @param pName
 */
codegen_container *codeGenCreateStruct(char *pName)
{
    if (pName == 0)
    {
        return 0;
    }
    size_t nameLen = strlen(pName);

    if (nameLen == 0)
    {
        return 0;
    }

    codegen_container *res = codeGenCreateContainer();

    if (res != 0)
    {
        res->type = CODEGEN_TYPE_STRUCT;
        void *pRes = setName(res, pName, nameLen);

        if (pRes == 0)
        {
            codeGenFree(res);
            return 0;
        }
    }

    return res;
}

/**
 * @param pStruct
 * @param pContainer
 */
int codeGenAddStructPropertyContainer(codegen_container *pStruct, codegen_container *pContainer)
{
    if ((pStruct == 0) || (pContainer == 0))
    {
        return ERROR_CODE_INVALID_ARGUMENTS;
    }

    codegen_container *lastSub = pStruct->sub;
    codegen_container *curSub = lastSub;

    while (curSub != 0)
    {
        lastSub = curSub;
        curSub = lastSub->next;
    }

    if (lastSub == 0)
    {
        pStruct->sub = pContainer;
    }
    else
    {
        lastSub->next = pContainer;
    }
    pContainer->refCount++;

    return 0;
}

/**
 * @param pStruct
 * @param type
 * @param name
 */
int codeGenAddStructProperty(codegen_container *pStruct, char *type, char *name)
{
    if ((pStruct == 0) || (type == 0) || (name == 0))
    {
        return 1;
    }
    if (pStruct->type != CODEGEN_TYPE_STRUCT)
    {
        return 2;
    }

    int res = 0;
    codegen_container *propRes = codeGenCreateTypedDeclaration(type, name, &res);

    if (propRes == 0)
    {
        return res;
    }

    return codeGenAddStructPropertyContainer(pStruct, propRes);
}

#define ERROR_CODE_OUTPUT_COPY_FAILED 10

int codeGenOutput(codegen_container *pInput, int options, char *output, size_t outputLen, size_t *resultLen)
{
    if ((pInput == 0) || (output == 0) || (outputLen == 0))
    {
        return ERROR_CODE_INVALID_ARGUMENTS;
    }
    printf("Sizes: container=%ld | ptr-cont=%ld\n",
           sizeof(struct codegen_container_type),
           sizeof(struct codegen_pointer_container_type));

    char *buffer = mallocZeroInit(16 * 1024);

    if (buffer == 0)
    {
        return ERROR_CODE_FAILED_CREATION;
    }
    char structText[1024];
    memset(structText, 0, 1024);
    int resSize;

    int isDummy = options & 0x1;
    size_t resultSize = 0;
    char *dst = output;
    size_t spaceLeft = outputLen;
    int res = 0;

    size_t resSizeGen = 0;

    switch (pInput->type)
    {
    case CODEGEN_TYPE_TYPE_GENERIC:
        printf("GEN: Generic\n");
        codegen_container *pNext = pInput->next;

        while (pNext != 0)
        {
            res = codeGenOutput(pNext, options, dst, spaceLeft, &resSizeGen);
            if (res != 0)
            {
                break;
            }
            if (spaceLeft < resSizeGen)
            {
                break;
            }
            dst += resSizeGen;
            spaceLeft -= resSizeGen;
            resultSize += resSizeGen;
            pNext = pNext->next;
        }
        break;
    case CODEGEN_TYPE_STRUCT:
        printf("GEN: struct\n");
        char *pText = getNamePtr(pInput);
        resSize = snprintf(structText, 1023, "struct %s\n{\n", pText);

        if (resSize > 0 && resSize < 1023)
        {
            resultSize += resSize + 4; // +};NLNL

            if (!isDummy)
            {
                if (resSize < spaceLeft)
                {
                    strncpy(dst, structText, resSize);
                    spaceLeft -= resSize;
                    dst += resSize;
                }
                else
                {
                    res = ERROR_CODE_OUTPUT_COPY_FAILED;
                    break;
                }

                if (spaceLeft > 4)
                {
                    *dst = '}';
                    ++dst;
                    *dst = ';';
                    ++dst;
                    *dst = '\n';
                    ++dst;
                    *dst = '\n';
                    ++dst;
                    spaceLeft -= 4;
                }
            }
        }
        else
        {
            printf("FAILED: struct.\n");
        }
        break;
    default:
        printf("Unknown type: %d\n", pInput->type);
    }

    // printf("FREE:Buffer...\n");
    free(buffer);
    // printf("FREE:Buffer...DONE\n");

    if (resultLen != 0)
    {
        *resultLen += resultSize;
    }
    return res;
}

#endif
