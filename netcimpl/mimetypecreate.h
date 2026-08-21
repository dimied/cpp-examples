#ifndef MIME_TYPE_CREATE
#define MIME_TYPE_CREATE

#include <string.h>
#include "../gencodeinc.h"

#define MASK_4BYTES (~(0x3))
#define MASK_8BYTES (~(0x7))

// #include <string.h>
int compare(const void *a, const void *b)
{
    if (a == b)
    {
        return 0;
    }

    const char *str1 = *(const char *const *)a;
    const char *str2 = *(const char *const *)b;

    return strcmp(str1, str2);
}

char *typedStruct[] = {"\nstruct typed_content_type\n",
                       "{\n",
                       "    unsigned int type;\n",
                       "    unsigned int offset;\n"
                       "};\n",
                       NULL};

char *typedStruct2[] = {"\nstruct typed_content2_type\n",
                        "{\n",
                        "    char type[8];\n",
                        "    char* content_type;\n"
                        "};\n",
                        NULL};

char *unknownType = "const char unknownType[] = \"text/plain\";\n\n";

char *funcText = "\n\
char* getMyMimeType(unsigned int type)\n\
{\n\
    if(type == 0)\n\
    {\n\
        return unknownType;\n\
    }\n\
    for(size_t i=0; i < NUM_MIME_TYPES; i++)\n\
    {\n\
        if(type == myMimeTypes[i].type)\n\
        {\n\
            return &allTypes[(myMimeTypes[i].offset&0xFFFF)];\n\
        }\n\
    }\n\
    return unknownType;\n\
}\n\
\n";

struct typed_content_type_dummy
{
    unsigned int type;
    unsigned int offset;
    // char *content_type;
};

struct typed_content_type2_dummy
{
    char type[8];
    char *pContentType;
};

size_t sizeOfLines(char **lines)
{
    size_t result = 0;
    char *pChar;
    while ((pChar = *lines) != 0)
    {
        result += strlen(pChar);
        lines++;
    }
    return result;
}

void freeMany(void **pointers, size_t sz)
{
    if (pointers == 0)
    {
        for (size_t i = 0; i < sz; i++)
        {
            free(pointers[i]);
            pointers[i] = 0;
        }
    }
}

// 2 arrays
//  - pointers to text
//  - (number of copies of longs, real string size)
//  Algo:
//  copy long and adjust pointer based on real size
int algo(char **strings, unsigned int *sizes, size_t entries, char *dst, size_t dstLen)
{
    size_t totalSize = 0;
    for (size_t i = 0; i < entries; i++)
    {
        totalSize += sizes[i];
    }
    if (totalSize > dstLen)
    {
        return 1;
    }
#if 0
    //Can we copy longs?
    if (totalSize+7 < dstLen) {

    }
#endif

    for (size_t i = 0; i < entries; i++)
    {
        char *p = strings[i];
        size_t n = (sizes[i] & 0xFFFF);
        strncpy(dst, p, n);
        dst += n;
        // TODO:copy longs
    }

    return 0;
}

char *testCodeGen()
{
    size_t outSize = 10 * 1024;

    char *cgResult = calloc(outSize, 1);

    if (cgResult == 0)
    {
        return 0;
    }

    codegen_container *allRes = codeGenCreateContainer();

    if (allRes == 0)
    {
        printf("FAILED: allRes\n");
        return cgResult;
    }
    allRes->type = CODEGEN_TYPE_TYPE_GENERIC;

    codegen_container *lastRes = allRes;

    codegen_container *structRes = codeGenCreateStruct("dada");

    if (structRes == 0)
    {
        printf("FAILED: struct\n");
        codeGenFree(allRes);
        return cgResult;
    }
    lastRes->next = structRes;
    lastRes = structRes;

    size_t resultLength = 0;

    printf("Out...\n");
    codeGenOutput(allRes, 0, cgResult, outSize, &resultLength);
    printf("Out:generated!\n");
    printAssignmentStats(NULL, 0, NULL);

    codeGenFree(allRes);

    return cgResult;
}

char *getContent(size_t *pSize)
{
    char *res = testCodeGen();

    if (res != 0 && pSize != 0)
    {
        *pSize = strlen(res);
    }
    return res;

#define SOME_PTR_COUNT 5
    void *pointers[SOME_PTR_COUNT] = {};
    memset(pointers, 0, sizeof(void *) * SOME_PTR_COUNT);

    printf("CT:size: %ld", sizeof(struct typed_content_type_dummy));
    printf("\tCT2:size: %ld", sizeof(struct typed_content_type2_dummy));
    printf("\n");
    size_t numEntries = sizeof(content_type_table) / sizeof(struct table_entry);
    printf("#entries: %ld\n", numEntries);

    char **entryPointers = (char **)malloc(numEntries * sizeof(char *));
    if (entryPointers == 0)
    {
        printf("ERROR: entry pointers\n");
        return 0;
    }
    pointers[0] = (void *)entryPointers;

    char **entryPointers2 = (char **)malloc(numEntries * sizeof(char *));
    if (entryPointers2 == 0)
    {
        printf("ERROR: entry pointers2\n");
        free(entryPointers);
        return 0;
    }
    pointers[1] = (void *)entryPointers2;

    size_t realSizeCT = 0, alignedSizeCT = 0, realSizeExt = 0, alignedSizeExt = 0;

    for (size_t i = 0; i < numEntries; i++)
    {
        char *p = (char *)content_type_table[i].extension;
        char *p2 = (char *)content_type_table[i].content_type;
        if (p == 0 || p2 == 0)
        {
            numEntries = i;
            break;
        }
        entryPointers[i] = p;
        entryPointers2[i] = p2;
    }
    qsort(entryPointers, numEntries, sizeof(entryPointers[0]), compare);
    qsort(entryPointers2, numEntries, sizeof(entryPointers2[0]), compare);

    int hasRepeatingCT = 0, longSized = 0;

    for (size_t i = 0; i < numEntries; i++)
    {
        if (i > 0 && (0 == strcmp(entryPointers[i], entryPointers[i - 1])))
        {
            printf("Repeating extension: %s\n", entryPointers[i]);
            continue;
        }
        size_t s = strlen(entryPointers[i]);
        realSizeExt += (s + 1);
        alignedSizeExt += (s + 3) & MASK_4BYTES;

        if (i > 0 && (0 == strcmp(entryPointers2[i], entryPointers2[i - 1])))
        {
            ++hasRepeatingCT;
            continue;
        }

        s = strlen(entryPointers2[i]);
        realSizeCT += (s + 1);
        alignedSizeCT += (s + 7) & MASK_8BYTES;

        if ((s & 0x7) == 0)
        {
            longSized++;
            // printf("long: %s\n", entryPointers2[i]);
            //  alignedSizeCT += s + 8;
        }
    }

    printf("Ext:sizes: %ld | aligned: %ld\n", realSizeExt, alignedSizeExt);
    printf("Type:sizes: %ld | aligned: %ld\n", realSizeCT, alignedSizeCT);
    printf("CT:repeating: %d | #8bytes: %d\n", hasRepeatingCT, longSized);
    size_t allocAlignedSizeCT = alignedSizeCT << 2;

    char *contentTypeText = (char *)malloc(allocAlignedSizeCT);

    if (contentTypeText == 0)
    {
        freeMany(pointers, SOME_PTR_COUNT);
        return 0;
    }
    pointers[2] = (void *)contentTypeText;
    memset(contentTypeText, 0, allocAlignedSizeCT);

    size_t *sizes = (size_t *)malloc(sizeof(size_t) * numEntries);
    if (sizes == 0)
    {
        freeMany(pointers, SOME_PTR_COUNT);
        return 0;
    }
    pointers[3] = (void *)sizes;
    memset(sizes, 0, sizeof(size_t) * numEntries);

    size_t *offsets = (size_t *)malloc(sizeof(size_t) * numEntries);
    if (offsets == 0)
    {
        freeMany(pointers, SOME_PTR_COUNT);
        return 0;
    }
    pointers[4] = (void *)offsets;
    memset(offsets, 0, sizeof(size_t) * numEntries);

    size_t dstOffset = 0;
    for (size_t i = 0; i < numEntries; i++)
    {
        if (i > 0 && (0 == strcmp(entryPointers2[i], entryPointers2[i - 1])))
        {
            sizes[i] = sizes[i - 1];
            offsets[i] = offsets[i - 1];
            continue;
        }
        offsets[i] = dstOffset;

        size_t s = 0;
        char *p = entryPointers2[i];
        while (*p != 0)
        {
            contentTypeText[dstOffset] = *p;
            dstOffset++;
            ++p;
            ++s;
        }
#if 0
        contentTypeText[dstOffset] = '\r';
        contentTypeText[dstOffset + 1] = '\n';
        dstOffset += 2;
        s += 2;
#endif
        sizes[i] = s;
        size_t alignedS = (s + 7) & MASK_8BYTES;
        if (alignedS == s)
        {
            alignedS += 8;
        }
        // aligned save
        while (s < alignedS)
        {
            contentTypeText[dstOffset] = ' ';
            ++dstOffset;
            ++s;
        }
    }

    for (size_t i = 0; i < numEntries; i++)
    {
        printf("%s ", entryPointers[i]);
    }
    printf("\n");

    printf("Lines:size: %ld\n", sizeOfLines(typedStruct));

    // printf("S:(%ld):\n'%s'\n", strlen(contentTypeText), contentTypeText);

    size_t size = 10000;

    char *result = malloc(size);
    if (result == 0)
    {
        freeMany(pointers, SOME_PTR_COUNT);
        return 0;
    }
    memset(result, 0, size);
    printf("Start writing ...\n");
    char *dst = result;
    size_t sl;

    char start[] = "#ifndef MIME_TYPE_FAST\n#define MIME_TYPE_FAST\n\n";
    char end[] = "\n#endif\n";

    sl = strlen(start);
    strcpy(result, start);
    result += sl;

    char defineText[100];
    memset(defineText, 0, 100);
    sprintf(defineText, "\n#define NUM_MIME_TYPES    (%ld)\n\n", numEntries);
    sl = strlen(defineText);
    strcpy(result, defineText);
    result += sl;

    sl = strlen(unknownType);
    strcpy(result, unknownType);
    result += sl;

    sl = strlen(funcText);
    printf("FUNC: %ld\n", sl);
    strcpy(result, funcText);
    result += sl;

    sl = strlen(end);
    strcpy(result, end);
    result += sl;

    freeMany(pointers, SOME_PTR_COUNT);
    *pSize = strlen(dst);

    return dst;
}

#endif