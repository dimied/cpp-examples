

void createAlignedHeadersText()
{
    size_t i;
    size_t alignedSize = 0, realSize = 0;
    char numHits = 0;
    char hits[NUM_HEADER_HANDLERS];
    memset(hits, 0, NUM_HEADER_HANDLERS);
    uint64_t sizes = 0;
    size_t minSize = 64, maxSize = 0;
    size_t sizeCounts[64];

    memset(sizeCounts, 0, sizeof(sizeCounts));

    for (i = 0; i < NUM_HEADER_HANDLERS; i++)
    {
        size_t s = strlen(headerHandlers[i].name);
        if (s == 0)
        {
            printf("Error: empty name @idx=%ld\n", i);
            continue;
        }
        size_t padding = (s & 0x7);

        if (padding == 0)
        {
            numHits++;
            hits[i] = numHits;
        }
        if (padding != 0)
        {
            padding = 8 - padding;
        }
        alignedSize += (s + padding);
        realSize += s + 1;

        if (minSize > s)
        {
            minSize = s;
        }
        if (maxSize < s)
        {
            maxSize = s;
        }

        if (s < 64)
        {
            sizes |= (0x1ul << (s - 1));
            sizeCounts[s]++;
        }
        else
        {
            printf("ERROR: size too large\n");
        }
    }

    printf("Sizes: real=%ld | aligned=%ld\n", realSize, alignedSize);

    if (numHits != 0)
    {
        printf("Hits:\n");

        for (i = 0; i < NUM_HEADER_HANDLERS; i++)
        {
            if (hits[i])
            {
                printf("|%s|\n", headerHandlers[i].name);
            }
        }
    }

    {
        printf("SIZES:");
        for (i = 0; i < 64; i++)
        {
            if (sizeCounts[i] != 0)
            {
                printf("%2ld|", i);
            }
        }
        printf("\n");
        printf("COUNT:");
        for (i = 0; i < 64; i++)
        {
            if (sizeCounts[i] != 0)
            {
                printf("%2ld|", sizeCounts[i]);
            }
        }
        printf("\n");
    }

    const uint64_t sizesBitmask = sizes;

    printf("\n//CODE\n");

    printf("\n#define HEADER_NAME_MIN_LEN (%ld)\n", minSize);
    printf("#define HEADER_NAME_MAX_LEN (%ld)\n", maxSize);
    printf("#define HEADER_SIZE_BITS_CONSTANT (%ldul)\n\n", sizesBitmask);
    {
        printf("//Sizes:bitmask:");
        size_t ts = 1;
        while (sizes != 0)
        {
            if (sizes & 0x1)
            {
                if (minSize == 0)
                {
                    minSize = ts;
                }
                printf(" %ld", ts);
            }
            sizes >>= 1;
            ts++;
        }
        printf("\n");
    }

    printf("uint64_t header_sizes_bits = %ldul;\n\n", sizesBitmask);

    printf("const char headerNamesAligned8[%ld] = {\n", alignedSize);

    size_t sz = 0;
    memset(sizeCounts, 0, sizeof(sizeCounts));

    uint64_t offsets[64];
    memset(offsets, 0, sizeof(offsets));

    char *names[64][8];
    memset(names, 0, sizeof(names));

    uint64_t values[64];
    memset(values, 0, sizeof(values));
    int hasError = 0;

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
        const uint64_t shiftIdx = sizeCounts[s];
        names[s][shiftIdx] = pStr;
        const uint64_t offset = (sz >> 3);
        if (offset > 255)
        {
            printf("//ERROR: value too large @%ld|%ld|%s|\n", i, offset, pStr);
            hasError++;
            break;
        }
        const size_t shift = shiftIdx << 3; //<<3: for bytes
        offsets[s] |= offset << shift;      //>>3: for aligned offsets
        values[s] |= i << shift;
        sizeCounts[s]++;

        for (sIdx = 0; sIdx < s; sIdx++)
        {
            printf("'%c'", pStr[sIdx]);
            sz++;
            if (sz < alignedSize)
            {
                printf(",");
            }
        }
        int sepAdded = 0;
        if (sz > 0 && ((sz & 0x7) == 0))
        {
            printf(" /**/");
            sepAdded++;
            printf("\n");
        }

        while (padding != 0)
        {
            printf("0");
            sz++;
            padding--;
            if (sz < alignedSize)
            {
                printf(",");
            }
        }

        if ((sepAdded == 0) && sz > 0 && ((sz & 0x7) == 0))
        {
            printf(" /**/");
            printf("\n");
        }
    }
    printf("};\n\n");

    if (hasError == 0)
    {
        printf("const uint64_t headerNamesOffsets8[%ld] = {\n", maxSize + 1);
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
                uint64_t off = offsets[i];

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

    printf("//CODE-END\n\n");
}