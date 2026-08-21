#ifndef GEN_CODE_IN_C
#define GEN_CODE_IN_C

#include <string.h>
#include "gencodemem.h"

#define CODEGEN_TYPE_TYPE_GENERIC 1
#define CODEGEN_TYPE_TYPE_DECL 2
#define CODEGEN_TYPE_ARRAY_TYPE_DECL 3
#define CODEGEN_TYPE_STRUCT 4

#define CODEGEN_TYPE_HEADER_GUARD 30

#define CODEGEN_TYPE_INT8_ARRAY 10
#define CODEGEN_TYPE_FIRST_ARRAY_TYPE CODEGEN_TYPE_INT8_ARRAY
#define CODEGEN_TYPE_CHAR_ARRAY CODEGEN_TYPE_INT8_ARRAY
#define CODEGEN_TYPE_UINT8_ARRAY 11
#define CODEGEN_TYPE_UCHAR_ARRAY CODEGEN_TYPE_UINT8_ARRAY
#define CODEGEN_TYPE_INT16_ARRAY 12
#define CODEGEN_TYPE_UINT16_ARRAY 13
#define CODEGEN_TYPE_SHORT_ARRAY CODEGEN_TYPE_INT16_ARRAY
#define CODEGEN_TYPE_USHORT_ARRAY CODEGEN_TYPE_UINT16_ARRAY
#define CODEGEN_TYPE_INT32_ARRAY 14
#define CODEGEN_TYPE_UINT32_ARRAY 15
#define CODEGEN_TYPE_INT64_ARRAY 16
#define CODEGEN_TYPE_UINT64_ARRAY 17
#define CODEGEN_TYPE_FLOAT_ARRAY 18
#define CODEGEN_TYPE_DOUBLE_ARRAY 19
#define CODEGEN_TYPE_LAST_ARRAY_TYPE CODEGEN_TYPE_DOUBLE_ARRAY

#define CODEGEN_IS_ARRAY_TYPE(TYPE) ((TYPE >= CODEGEN_TYPE_FIRST_ARRAY_TYPE) && (TYPE <= CODEGEN_TYPE_LAST_ARRAY_TYPE))

#define ERROR_CODE_INVALID_ARGUMENTS 1
#define ERROR_CODE_FAILED_CREATION 2
#define ERROR_CODE_FAILED_COPY 3
#define ERROR_CODE_OVERFLOW 10

#define CODEGEN_OPTION_INLINE_NAME (0x1)
#define CODEGEN_OPTION_INLINE_TYPE (0x2)
#define CODEGEN_OPTION_INLINE_ARR_VALUE (0x4)
#define CODEGEN_OPTION_INLINE_SINGLE_SUB (0x8)

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

#define CODEGEN_POINTER_CONTAINER_NUM_ELEMENTS 7
/**
 * Linked list storing pointers
 */
struct codegen_pointer_container_type
{
    // unsigned int type;
    struct codegen_pointer_container_type *next;
    void *content[CODEGEN_POINTER_CONTAINER_NUM_ELEMENTS];
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
    // CharArrayOrPointer comment;
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
            for (int i = 0; i < CODEGEN_POINTER_CONTAINER_NUM_ELEMENTS; i++)
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
                for (int i = 0; i < CODEGEN_POINTER_CONTAINER_NUM_ELEMENTS; i++)
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

codegen_container *codeGenCreateHeaderGuard(char *pName)
{
    codegen_container *pDefine = codeGenCreateStruct(pName);
    if (pDefine != 0)
    {
        pDefine->type = CODEGEN_TYPE_HEADER_GUARD;
    }
    return pDefine;
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

typedef struct
{
    //char *dst;
    //size_t spaceLeft;
    size_t resultSize;
    //size_t resSizeGen;
} CodeGenOutputState;

typedef struct
{
    size_t outputLen;
    size_t used;
    char *output;
} CodeGenOutput;

int initCodeGenOutput(CodeGenOutput *pOutput)
{
    if (pOutput != 0)
    {
        memset(pOutput, 0, sizeof(CodeGenOutput));
        return 0;
    }
    return ERROR_CODE_INVALID_ARGUMENTS;
}

#if 0
char *getDest(CodeGenOutput *pOutput)
{
    if (pOutput != 0)
    {
        if (pOutput->used < pOutput->outputLen)
        {
            return pOutput->output + pOutput->used;
        }
    }
    return 0;
}
#endif

int writeToOutput(CodeGenOutput *pOutput, char *pData, size_t len, int options)
{
    if (pOutput == 0)
    {
        return ERROR_CODE_INVALID_ARGUMENTS;
    }

    if (options & 0x1)
    {
        pOutput->used += len;
        return 0;
    }

    if (pOutput->outputLen > pOutput->used)
    {
        size_t diff = pOutput->outputLen - pOutput->used;
        if (len < diff)
        {
            char *dst = pOutput->output + pOutput->used;
            memcpy(dst, pData, len);
            pOutput->used += len;
            return 0;
        }
    }
    return ERROR_CODE_OVERFLOW;
}

int codeGenOutput(codegen_container *pInput, int options, CodeGenOutput *pOutput, size_t *resultLen);

int codeGenOutputForSub(codegen_container *pInput, int options, CodeGenOutput *pOutput, size_t *resultLen, CodeGenOutputState *pOutputState)
{
    if ((pInput == 0) || (pOutput == 0))
    {
        return ERROR_CODE_INVALID_ARGUMENTS;
    }

    void *pSub = pInput->sub.pAny;
    int res = 0;

    if (pSub != 0)
    {
        if (HAS_OPTION_SINGLE_SUB(pInput->options))
        {
            res = codeGenOutput(pSub, options, pOutput, &pOutputState->resultSize);
        }
        else
        {
            struct codegen_pointer_container_type *pSubs = (struct codegen_pointer_container_type *)pSub;

            while (pSubs != 0)
            {
                int doIterate = 0;

                for (int i = 0; i < CODEGEN_POINTER_CONTAINER_NUM_ELEMENTS; i++)
                {
                    struct codegen_container_type *pSubItem = (struct codegen_container_type *)pSubs->content[i];
                    if (pSubItem != 0)
                    {
                        //pOutputState->resSizeGen = 0;

                        res = codeGenOutput(pSubItem, options, pOutput, &pOutputState->resultSize);
                        if (res != 0)
                        {
                            doIterate++;
                            break;
                        }
                    }
                }
                if (doIterate)
                {
                    break;
                }

                pSubs = pSubs->next;
            }
        }
    }
    return res;
}

char closingBracket[] = "};\n";
char closingBracketNL[] = "};\n\n";

int codeGenOutput(codegen_container *pInput, int options, CodeGenOutput *pOutput, size_t *resultLen)
{
    if ((pInput == 0) || (pOutput == 0))
    {
        return ERROR_CODE_INVALID_ARGUMENTS;
    }

    char structText[1024];
    memset(structText, 0, 1024);
    int resSize;

    int res = 0;

    CodeGenOutputState state;
    // state.spaceLeft = pOutput->outputLen;
    memset(&state, 0, sizeof(state));
    int outputRes = 0;
    char *pText;

    switch (pInput->type)
    {
    case CODEGEN_TYPE_TYPE_GENERIC:
        printf("GEN: Generic\n");
        res = codeGenOutputForSub(pInput, options, pOutput, resultLen, &state);
        printf("GEN: Generic: %d\n", res);
        break;
    case CODEGEN_TYPE_HEADER_GUARD:
        printf("GEN: header guard\n");

        pText = getNamePtr(pInput);
        resSize = snprintf(structText, 1023, "#ifndef %s\n#define %s\n\n", pText, pText);

        if (resSize > 0 && resSize < 1023)
        {
            outputRes = writeToOutput(pOutput, structText, resSize, options);

            if (outputRes != 0)
            {
                return outputRes;
            }

            res = codeGenOutputForSub(pInput, options, pOutput, resultLen, &state);

            char *closing = "\n#endif\n\n";

            // TODO: create internal entries
            outputRes = writeToOutput(pOutput, closing, strlen(closing), options);
            if (outputRes != 0)
            {
                return outputRes;
            }
        }
        else
        {
            res = 100;
            printf("FAILED: struct.\n");
        }
        printf("GEN: header guard: %d\n", res);
        break;
    case CODEGEN_TYPE_STRUCT:
        printf("GEN: struct\n");
        pText = getNamePtr(pInput);
        resSize = snprintf(structText, 1023, "struct %s\n{\n", pText);

        if (resSize > 0 && resSize < 1023)
        {
            outputRes = writeToOutput(pOutput, structText, resSize, options);

            if (outputRes != 0)
            {
                return outputRes;
            }
            char *closing = closingBracketNL;

            // TODO: create internal entries
            outputRes = writeToOutput(pOutput, closing, strlen(closing), options);
            if (outputRes != 0)
            {
                return outputRes;
            }
        }
        else
        {
            res = 100;
            printf("FAILED: struct.\n");
        }
        break;
    default:
        printf("Unknown type: %d\n", pInput->type);
    }

    if (resultLen != 0)
    {
        *resultLen += state.resultSize;
    }

    printf("GEN:RES: %d\n", res);

    return res;
}

#endif
