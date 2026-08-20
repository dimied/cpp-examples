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

struct codegen_container_type
{
    unsigned int type;
    unsigned int options;
    unsigned int refCount;
    size_t arrCount;
    char *pName;
    char *pType;
    char *arrInitValue;
    struct codegen_container_type *next;
    struct codegen_container_type *sub;
};

typedef struct codegen_container_type codegen_container;

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

/**
 * @param pContainer
 */
int codeGenFree(codegen_container *pContainer)
{
    if (pContainer == 0)
    {
        return ERROR_CODE_INVALID_ARGUMENTS;
    }

    printf("Start:codeGenFree\n");

    if (pContainer->pName != 0)
    {
        printf("FREE:name...");
        free(pContainer->pName);
        pContainer->pName = 0;
        printf("DONE\n");
    }
    if (pContainer->pType != 0)
    {
        printf("FREE:type...");
        free(pContainer->pType);
        pContainer->pType = 0;
        printf("DONE\n");
    }
    if (pContainer->arrInitValue != 0)
    {
        printf("FREE:arrInit...");
        free(pContainer->arrInitValue);
        pContainer->arrInitValue = 0;
        printf("DONE\n");
    }

    if (pContainer->next != 0)
    {
        printf("FREE:next...\n");
        codeGenFree(pContainer->next);
        pContainer->next = 0;
        printf("DONE\n");
    }

    printf("FREE:CONTAINER...\n");
    fflush(stdout);
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

    size_t nameLen = strlen(name);
    char *typeCopy = (char *)mallocZeroInit(nameLen + 1);
    if (typeCopy == 0)
    {
        codeGenSetResult(pRes, ERROR_CODE_FAILED_COPY);
        codeGenFree(propRes);
        return 0;
    }
    strncpy(typeCopy, type, nameLen);

    nameLen = strlen(name);
    char *nameCopy = (char *)mallocZeroInit(nameLen + 1);
    if (nameCopy == 0)
    {
        codeGenSetResult(pRes, ERROR_CODE_FAILED_COPY);
        free(typeCopy);
        codeGenFree(propRes);
        return 0;
    }
    strncpy(nameCopy, name, nameLen);

    propRes->type = CODEGEN_TYPE_TYPE_DECL;
    propRes->pName = nameCopy;
    propRes->pType = typeCopy;

    return propRes;
}

codegen_container *codeGenCreateArrayDeclaration(char *type, char *name, size_t size, char *arrInitValue, int *pRes)
{
    codegen_container *propRes = codeGenCreateTypedDeclaration(type, name, pRes);
    if (propRes == 0)
    {
        return propRes;
    }

    if (arrInitValue != 0)
    {
        size_t nameLen = strlen(arrInitValue);
        char *copy = (char *)mallocZeroInit(nameLen + 1);
        if (copy == 0)
        {
            codeGenSetResult(pRes, ERROR_CODE_FAILED_COPY);
            codeGenFree(propRes);
            return 0;
        }
        strncpy(copy, arrInitValue, nameLen);
        propRes->arrInitValue = copy;
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

    char *nameCopy = (char *)mallocZeroInit(nameLen + 1);
    if (nameCopy == 0)
    {
        return 0;
    }
    strncpy(nameCopy, pName, nameLen);

    codegen_container *res = codeGenCreateContainer();

    if (res != 0)
    {
        res->type = CODEGEN_TYPE_STRUCT;
        res->pName = nameCopy;
    }
    else
    {
        free(nameCopy);
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
        resSize = snprintf(structText, 1023, "struct %s\n{\n", pInput->pName);

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

    printf("FREE:Buffer...\n");
    free(buffer);
    printf("FREE:Buffer...DONE\n");

    if (resultLen != 0)
    {
        *resultLen += resultSize;
    }
    return res;
}

#endif
