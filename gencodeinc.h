#ifndef GEN_CODE_IN_C
#define GEN_CODE_IN_C

#include <string.h>
#include "gencodemem.h"

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
#define CODEGEN_OPTION_INLINE_SINGLE_NEXT (0x8)
#define CODEGEN_OPTION_INLINE_SINGLE_SUB (0x100)

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

#define SET_OPTION_SINGLE_SUB(OPTS) (OPTS |= CODEGEN_OPTION_INLINE_SINGLE_SUB)
#define UNSET_OPTION_SINGLE_SUB(OPTS) (OPTS &= (~CODEGEN_OPTION_INLINE_SINGLE_SUB))
#define HAS_OPTION_SINGLE_SUB(OPTS) ((OPTS &= CODEGEN_OPTION_INLINE_SINGLE_SUB) != 0)
#define HAS_NO_OPTION_SINGLE_SUB(OPTS) ((OPTS & CODEGEN_OPTION_INLINE_SINGLE_SUB) == 0)

#define SET_OPTION_SINGLE_NEXT(OPTS) (OPTS |= CODEGEN_OPTION_INLINE_SINGLE_NEXT)
#define UNSET_OPTION_SINGLE_NEXT(OPTS) (OPTS &= (~CODEGEN_OPTION_INLINE_SINGLE_NEXT))
#define HAS_OPTION_SINGLE_NEXT(OPTS) ((OPTS &= CODEGEN_OPTION_INLINE_SINGLE_NEXT) != 0)
#define HAS_NO_OPTION_SINGLE_NEXT(OPTS) ((OPTS & CODEGEN_OPTION_INLINE_SINGLE_NEXT) == 0)

/**
 * Linked list storing pointers
 */
struct codegen_pointer_container_type
{
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
    //CharArrayOrPointer comment;
    union
    {
        void *pAny;
        struct codegen_pointer_container_type *manySubs;
        struct codegen_container_type *sub;
    } sub;
};

typedef struct codegen_container_type codegen_container;

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

    printf("Sizes: container=%ld | ptr-cont=%ld\n",
           sizeof(struct codegen_container_type),
           sizeof(struct codegen_pointer_container_type));

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

#define ALLOC_TYPE_UNKNOWN 0
#define ALLOC_TYPE_STRING 1
#define ALLOC_TYPE_CONTAINER 2
#define ALLOC_TYPE_PTR_CONTAINER 3
#define ALLOC_TYPE_STRING_BUFFER 4

#define ALLOC_SOURCE_UNKNOWN 0
#define ALLOC_SOURCE_CONTAINER 1
#define ALLOC_SOURCE_NAME 2
#define ALLOC_SOURCE_TYPE 3
#define ALLOC_SOURCE_ARR_INIT_VALUE 4
#define ALLOC_SOURCE_OUTPUT 5

void initGenCodeInC()
{
    initMem();
    memSetTypeName(ALLOC_TYPE_UNKNOWN, "unknown");
    memSetTypeName(ALLOC_TYPE_STRING, "string");
    memSetTypeName(ALLOC_TYPE_CONTAINER, "container");
    memSetTypeName(ALLOC_TYPE_PTR_CONTAINER, "ptr-container");
    memSetTypeName(ALLOC_TYPE_STRING_BUFFER, "buffer(string)");

    memSetSourceName(ALLOC_SOURCE_UNKNOWN, "unknown");
    memSetSourceName(ALLOC_SOURCE_CONTAINER, "container");
    memSetSourceName(ALLOC_SOURCE_NAME, "setName");
    memSetSourceName(ALLOC_SOURCE_TYPE, "setType");
    memSetSourceName(ALLOC_SOURCE_ARR_INIT_VALUE, "setArrInit");
    memSetSourceName(ALLOC_SOURCE_OUTPUT, "output");
}
/**
 *
 */
codegen_container *codeGenCreateContainer()
{
    return (codegen_container *)callocMemory(sizeof(codegen_container), ALLOC_TYPE_CONTAINER, ALLOC_SOURCE_CONTAINER);
}

char *getNamePtr(codegen_container *pContainer)
{
    if (pContainer == 0)
    {
        return 0;
    }
    return HAS_OPTION_INLINE_NAME(pContainer->options) ? pContainer->name.name : pContainer->name.pName;
}

void freeNameAttribute(codegen_container *pContainer)
{
    if (HAS_NO_OPTION_INLINE_NAME(pContainer->options) && pContainer->name.pName != (char *)0)
    {
        freeMemory(pContainer->name.pName);
        pContainer->name.pName = 0;
    }
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
        char *nameCopy = (char *)callocMemory(nameLen + 1, ALLOC_TYPE_STRING, ALLOC_SOURCE_NAME);
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

void freeTypeNameAttribute(codegen_container *pContainer)
{
    if (HAS_NO_OPTION_INLINE_TYPE(pContainer->options) && pContainer->typeName.pName != (char *)0)
    {
        freeMemory(pContainer->typeName.pName);
        pContainer->typeName.pName = 0;
    }
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
        char *nameCopy = (char *)callocMemory(nameLen + 1, ALLOC_TYPE_STRING, ALLOC_SOURCE_TYPE);
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

void freeArrInitValueAttribute(codegen_container *pContainer)
{
    if (HAS_NO_OPTION_INLINE_ARR_VALUE(pContainer->options) && pContainer->arrInitValue.pName != (char *)0)
    {
        freeMemory(pContainer->arrInitValue.pName);
        pContainer->arrInitValue.pName = 0;
    }
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
        char *nameCopy = (char *)callocMemory(nameLen + 1, ALLOC_TYPE_STRING, ALLOC_SOURCE_ARR_INIT_VALUE);
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

int codeGenAddSub(codegen_container *pContainer, void *pSub)
{
    if ((pContainer == 0) || (pSub == 0))
    {
        return ERROR_CODE_INVALID_ARGUMENTS;
    }

    void *oldP = pContainer->sub.pAny;

    if (oldP == 0)
    {
        pContainer->sub.pAny = pSub;
        SET_OPTION_SINGLE_SUB(pContainer->options);
    }
    else
    {
        struct codegen_pointer_container_type *pSubs = (struct codegen_pointer_container_type *)oldP;

        if (HAS_OPTION_SINGLE_SUB(pContainer->options))
        {
            pSubs = callocMemory(sizeof(struct codegen_pointer_container_type), ALLOC_TYPE_PTR_CONTAINER, 0);
            if (pSubs != 0)
            {
                pSubs->content[0] = oldP;
                pSubs->content[1] = pSub;
                pContainer->sub.manySubs = pSubs;
                UNSET_OPTION_SINGLE_SUB(pContainer->options);
                return 0;
            }
            return ERROR_CODE_FAILED_CREATION;
        }
        UNSET_OPTION_SINGLE_SUB(pContainer->options);
        struct codegen_pointer_container_type *pPrevSubs = pSubs;

        while (pSubs != 0)
        {
            pPrevSubs = pSubs;

            // search for free space
            // We don't check for identical values
            for (int i = 0; i < 6; i++)
            {
                if (pSubs->content[i] == 0)
                {
                    pSubs->content[i] = pSub;
                    return 0;
                }
            }
            pSubs = pSubs->next;
        }
        // No free space found
        pSubs = callocMemory(sizeof(struct codegen_pointer_container_type), ALLOC_TYPE_PTR_CONTAINER, 0);
        if (pSubs != 0)
        {
            pSubs->content[0] = pSub;
            pPrevSubs->next = pSubs;
            return 0;
        }
        return ERROR_CODE_FAILED_CREATION;
    }

    return 0;
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

    freeNameAttribute(pContainer);
    freeTypeNameAttribute(pContainer);
    freeArrInitValueAttribute(pContainer);

    if (pContainer->sub.pAny != 0)
    {
        if (HAS_OPTION_SINGLE_SUB(pContainer->options))
        {
            codeGenFree(pContainer->sub.sub);
            pContainer->sub.pAny = 0;
        }
        else
        {
            struct codegen_pointer_container_type *pSubs = pContainer->sub.manySubs;
            struct codegen_pointer_container_type *pNext;

            while (pSubs != 0)
            {
                for (int i = 0; i < 6; i++)
                {
                    void *pSubItem = pSubs->content[i];

                    if (pSubItem != 0)
                    {
                        codeGenFree((struct codegen_container_type *)pSubItem);
                        pSubs->content[i] = 0;
                    }
                }

                pNext = pSubs->next;

                freeMemory(pSubs);

                pContainer->sub.pAny = pNext;
                pSubs = pNext;
            }
        }
        UNSET_OPTION_SINGLE_SUB(pContainer->options);
    }

    freeMemory(pContainer);

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

    int res = codeGenAddSub(pStruct, pContainer);

    if (res != 0)
    {
        return res;
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

    // char *buffer = callocMemory(16 * 1024, ALLOC_TYPE_STRING_BUFFER, ALLOC_SOURCE_OUTPUT);
#if 0
    if (buffer == 0)
    {
        return ERROR_CODE_FAILED_CREATION;
    }
#endif
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
        void *pSub = pInput->sub.pAny;

        if (pSub != 0)
        {
            if (HAS_OPTION_SINGLE_SUB(pInput->options))
            {
                res = codeGenOutput(pSub, options, dst, spaceLeft, &resSizeGen);
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
            }
            else
            {
                struct codegen_pointer_container_type *pSubs = (struct codegen_pointer_container_type *)pSub;
                int doIterate = 0;
                while (pSubs != 0 && doIterate == 0)
                {
                    for (int i = 0; i < 6; i++)
                    {
                        struct codegen_container_type *pSubItem = (struct codegen_container_type *)pSubs->content[i];
                        if (pSubItem != 0)
                        {
                            resSizeGen = 0;

                            res = codeGenOutput(pSubItem, options, dst, spaceLeft, &resSizeGen);
                            if (res != 0)
                            {
                                doIterate++;
                                break;
                            }
                            if (spaceLeft < resSizeGen)
                            {
                                doIterate++;
                                break;
                            }
                            dst += resSizeGen;
                            spaceLeft -= resSizeGen;
                            resultSize += resSizeGen;
                        }
                    }

                    pSubs = pSubs->next;
                }
            }
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

    // freeMemory(buffer);

    if (resultLen != 0)
    {
        *resultLen += resultSize;
    }

    printf("GEN:RES: %d\n", res);

    return res;
}

#endif
