#ifndef HTTP_PARSER_DEV_HELPER
#define HTTP_PARSER_DEV_HELPER

#include "test_string_buffer.h"

typedef struct DevAlignedHeadersText
{
    size_t alignedSize;
    size_t realSize;
    size_t minSize;
    size_t maxSize;
    uint64_t sizes;
    int numHits;
    char hits[NUM_HEADER_HANDLERS];
    size_t sizeCounts[64];
    uint64_t offsets[64];
    char *names[64][8];
    uint64_t values[64];
} DevAlignedHeadersText;

int getAlignedHeadersTextStats(DevAlignedHeadersText *state)
{
    size_t i;
    memset(state, 0, sizeof(DevAlignedHeadersText));
    state->minSize = 64;

    for (i = 0; i < NUM_HEADER_HANDLERS; i++)
    {
        size_t s = strlen(headerHandlers[i].name);
        if (s == 0)
        {
            printf("Error: empty name @idx=%ld\n", i);
            return 1;
        }
        size_t padding = (s & 0x7);

        if (padding == 0)
        {
            state->numHits++;
            state->hits[i] = state->numHits;
        }
        if (padding != 0)
        {
            padding = 8 - padding;
        }
        state->alignedSize += (s + padding);
        state->realSize += s + 1;

        if (state->minSize > s)
        {
            state->minSize = s;
        }
        if (state->maxSize < s)
        {
            state->maxSize = s;
        }

        if (s < 64)
        {
            state->sizes |= (0x1ul << (s - 1));
            state->sizeCounts[s]++;
        }
        else
        {
            printf("ERROR: size too large\n");
            return 1;
        }
    }

    return 0;
}

void showAlignedStrings(DevAlignedHeadersText *state)
{
    if (state->numHits != 0)
    {
        printf("Hits:\n");
        size_t i;
        for (i = 0; i < NUM_HEADER_HANDLERS; i++)
        {
            if (state->hits[i])
            {
                printf("|%s|\n", headerHandlers[i].name);
            }
        }
    }
}

void showSizesAndCounts(DevAlignedHeadersText *state)
{
    printf("//SIZES:");
    size_t i;
    for (i = 0; i < 64; i++)
    {
        if (state->sizeCounts[i] != 0)
        {
            printf("%2ld|", i);
        }
    }
    printf("\n");
    printf("//COUNT:");
    for (i = 0; i < 64; i++)
    {
        if (state->sizeCounts[i] != 0)
        {
            printf("%2ld|", state->sizeCounts[i]);
        }
    }
    printf("\n");
}

char *generateTextForConstants(DevAlignedHeadersText *state)
{
    char buffer[128];
    memset(buffer, 0, 128);

    char *result = (char *)calloc(1024, 1);
    if (result == 0)
    {
        return 0;
    }

    const uint64_t sizesBitmask = state->sizes;

    char *pResult = result;

    int s = sprintf(buffer, "\n#define HEADER_NAME_MIN_LEN (%ld)\n", state->minSize);
    memcpy(pResult, buffer, s);
    memset(buffer, 0, s);
    pResult += s;

    s = sprintf(buffer, "#define HEADER_NAME_MAX_LEN (%ld)\n", state->maxSize);
    memcpy(pResult, buffer, s);
    memset(buffer, 0, s);
    pResult += s;

    s = sprintf(buffer, "#define HEADER_SIZE_BITS_CONSTANT (%ldul)\n\n", sizesBitmask);
    memcpy(pResult, buffer, s);
    memset(buffer, 0, s);
    pResult += s;

    {
        s = sprintf(buffer, "//Sizes:bitmask:");
        memcpy(pResult, buffer, s);
        memset(buffer, 0, s);
        pResult += s;

        size_t ts = 1;
        uint64_t sizes = state->sizes;
        while (sizes != 0)
        {
            if (sizes & 0x1)
            {

                s = sprintf(buffer, " %ld", ts);
                memcpy(pResult, buffer, s);
                memset(buffer, 0, s);
                pResult += s;
            }
            sizes >>= 1;
            ts++;
        }
        s = sprintf(buffer, "\n");
        memcpy(pResult, buffer, s);
        memset(buffer, 0, s);
        pResult += s;
    }

    s = sprintf(buffer, "uint64_t header_sizes_bits = %ldul;\n\n", sizesBitmask);
    memcpy(pResult, buffer, s);

    return result;
}

char *generateTextForAlignedValues(DevAlignedHeadersText *state)
{
    size_t totalSize = 0;

    TestStringBuffer string_buffer;

    char buffer[256];
    memset(buffer, 0, 256);

    clearTestStringBuffer(&string_buffer);

    int runs = 2;
    // size_t sizeCounts[64];
    int hasError = 0;

    while (runs != 0 && hasError == 0)
    {
        int r = initTestStringBuffer(&string_buffer, totalSize);
        if (0 != r)
        {
            printf("//ERROR: no string buffer.(CODE:%d)\n", r);
            break;
        }
        runs--;

        size_t i;
        r = sprintf(buffer, "const char headerNamesAligned8[%ld] = {\n", state->alignedSize);
        int addRes = addToTestStringBuffer(&string_buffer, buffer, r);
        if (addRes != 0)
        {
            printf("//ERROR: adding to string buffer.(CODE:%d)\n", r);
            hasError++;
            break;
        }
        memset(buffer, 0, r);

        size_t sz = 0;
        memset(state->sizeCounts, 0, sizeof(state->sizeCounts));
        memset(state->offsets, 0, sizeof(state->offsets));
        memset(state->names, 0, sizeof(state->names));
        memset(state->values, 0, sizeof(state->values));

        for (i = 0; i < NUM_HEADER_HANDLERS; i++)
        {
            char *pStr = headerHandlers[i].name;
            const size_t s = strlen(pStr);
            size_t padding = (s & 0x7);

            if (padding != 0)
            {
                padding = 8 - padding;
            }

            size_t sIdx;

            if (sz % 8 != 0)
            {
                printf("//ERROR: alignment @%ld|%s|\n", i, pStr);
                hasError++;
                break;
            }
            const uint64_t shiftIdx = state->sizeCounts[s];
            state->names[s][shiftIdx] = pStr;
            const uint64_t offset = (sz >> 3);
            if (offset > 255)
            {
                printf("//ERROR: value too large @%ld|%ld|%s|\n", i, offset, pStr);
                hasError++;
                break;
            }
            const size_t shift = shiftIdx << 3;   //<<3: for bytes
            state->offsets[s] |= offset << shift; //>>3: for aligned offsets
            state->values[s] |= i << shift;
            state->sizeCounts[s]++;

            for (sIdx = 0; hasError == 0 && sIdx < s; sIdx++)
            {

                int len = sprintf(buffer, "'%c'", pStr[sIdx]);
                addRes = addToTestStringBuffer(&string_buffer, buffer, len);
                if (0 != addRes)
                {
                    printf("//ERROR: add symbol\n");
                    hasError++;
                    break;
                }

                memset(buffer, 0, len);

                sz++;
                if (sz < state->alignedSize)
                {
                    addRes = addToTestStringBuffer(&string_buffer, ",", 1);
                    if (0 != addRes)
                    {
                        printf("//ERROR: add comma\n");
                        hasError++;
                        break;
                    }
                }
            }
            int sepAdded = 0;
            if (sz > 0 && ((sz & 0x7) == 0))
            {
                sepAdded++;
                addRes = addToTestStringBuffer(&string_buffer, " /**/\n", strlen(" /**/\n"));
                if (0 != addRes)
                {
                    printf("//ERROR: add line end(1)\n");
                    hasError++;
                    break;
                }
            }

            while (padding != 0)
            {
                addRes = addToTestStringBuffer(&string_buffer, "0", 1);
                if (0 != addRes)
                {
                    printf("//ERROR: add 0\n");
                    hasError++;
                    break;
                }
                sz++;
                padding--;

                if (sz < state->alignedSize)
                {
                    addRes = addToTestStringBuffer(&string_buffer, ",", 1);
                    if (0 != addRes)
                    {
                        printf("//ERROR: add comma(2)\n");
                        hasError++;
                        break;
                    }
                }
            }

            if ((sepAdded == 0) && sz > 0 && ((sz & 0x7) == 0))
            {
                addRes = addToTestStringBuffer(&string_buffer, " /**/\n", strlen(" /**/\n"));
                if (0 != addRes)
                {
                    printf("//ERROR: add line end(2)\n");
                    hasError++;
                    break;
                }
            }
        }
        addRes = addToTestStringBuffer(&string_buffer, "};\n\n", strlen("};\n\n"));
        if (0 != addRes)
        {
            printf("//ERROR: line end\n");
            hasError++;
            break;
        }
        // printf("//TOTAL-SIZE:%ld\n", string_buffer.used);
        totalSize = string_buffer.used;
    }

    if (hasError == 0)
    {
        return string_buffer.data;
    }
    printf("//ERROR: unknown\n");
    free(string_buffer.data);
    return 0;
}

char *generateTextForNameOffsets(DevAlignedHeadersText *state)
{
    TestStringBuffer string_buffer;

    char buffer[256];
    memset(buffer, 0, 256);

    clearTestStringBuffer(&string_buffer);

    printf("const uint64_t headerNamesOffsets8[%ld] = {\n", state->maxSize + 1);
    size_t j;
    size_t i;
    for (i = 0; i < state->maxSize + 1; i++)
    {
        char comma = (i < state->maxSize) ? ',' : ' ';

        const size_t c = state->sizeCounts[i];
        if (c == 0)
        {
            printf("  0%c /*size:%ld*/\n", comma, i);
        }
        else
        {
#if 1
            uint64_t off = state->offsets[i];

            printf("  (");
            printf("(%ld)", off & 0xFF);
            int shift = 8;
            off >>= 8;
            while (off != 0)
            {
                uint64_t o = off & 0xFF;

                printf("|((%ldul)<<%d)", o, shift);

                off >>= 8;
                shift += 8;
            }
            printf(")%c", comma);
#else
            printf("%ld%c ", offsets[i], comma);
#endif
            printf("/*size:%ld|", i);
            for (j = 0; j < c - 1; j++)
            {
                printf("%s,", state->names[i][j]);
            }
            printf("%s", state->names[i][j]);
            printf("*/\n");
        }
    }
    printf("};\n\n");

#if 0


    
    if (hasError == 0)
    {
        printf("const uint64_t headerNamesValues8[%ld] = {\n", maxSize + 1);
        size_t j;
        for (i = 0; i < maxSize + 1; i++)
        {
            char comma = (i < maxSize) ? ',' : ' ';

            const size_t c = sizeCounts[i];
            if (c == 0)
            {
                printf("0%c /*size:%ld*/\n", comma, i);
            }
            else
            {
#if 1
                uint64_t off = values[i];

                printf("(");
                printf("(%ld)", off & 0xFF);
                int shift = 8;
                off >>= 8;
                while (off != 0)
                {
                    uint64_t o = off & 0xFF;

                    printf("|((%ldul)<<%d)", o, shift);

                    off >>= 8;
                    shift += 8;
                }
                printf(")%c", comma);
#else
                printf("%ld%c ", offsets[i], comma);
#endif
                printf("/*size:%ld|", i);
                for (j = 0; j < c - 1; j++)
                {
                    printf("%s,", names[i][j]);
                }
                printf("%s", names[i][j]);
                printf("*/\n");
            }
        }
        printf("};\n\n");
    }
#endif
    return 0;
}

void createAlignedHeadersText()
{
    DevAlignedHeadersText state;
    memset(&state, 0, sizeof(DevAlignedHeadersText));

    if (0 != getAlignedHeadersTextStats(&state))
    {
        return;
    }

    printf("Sizes: real=%ld | aligned=%ld\n", state.realSize, state.alignedSize);

    showAlignedStrings(&state);
    showSizesAndCounts(&state);

    printf("\n//CODE\n");

    char *code = generateTextForConstants(&state);
    if (code == 0)
    {
        printf("//ERROR: failed to generate code for constants\n");
        return;
    }
    printf("%s", code);
    free(code);

    code = generateTextForAlignedValues(&state);
    if (code == 0)
    {
        printf("//ERROR: failed to generate array code\n");
        return;
    }
    printf("%s", code);
    free(code);

#if 0
    code = generateTextForNameOffsets(&state);
    if (code == 0)
    {
        printf("//ERROR: failed to generate name offsets\n");
        return;
    }
    printf("%s", code);
    free(code);
#endif
    printf("//CODE-END\n\n");
}

#endif