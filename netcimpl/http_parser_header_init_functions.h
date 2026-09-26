#ifndef HTTP_PARSER_HEADER_INIT_FUNCTIONS
#define HTTP_PARSER_HEADER_INIT_FUNCTIONS

#define IMPL_GET_HEADER_NAME_BY_IDX(IDX) (headerHandlers[IDX].name)

#ifndef INIT_HEADER_DATA_VALIDATE
#define INIT_HEADER_DATA_VALIDATE 1
#endif

// #define INIT_HEADER_DATA_
struct ps
{
    char *name;
    size_t size;
    int i;
};

int init_cmp_by_size_and_name(const void *a, const void *b)
{
    const struct ps *pA = (struct ps *)a;
    const struct ps *pB = (struct ps *)b;

    if (pA->size < pB->size)
    {
        return -1;
    }
    else if (pA->size > pB->size)
    {
        return 1;
    }
    return strcmp(pA->name, pB->name);
    // return 0;
}

#define INIT_HEADER_DATA_ERROR_HEADER_NAMES_SIZE_MISMATCH 1
#define INIT_HEADER_DATA_ERROR_HEADER_NAMES_LC_SIZE_MISMATCH 2

int init_header_data()
{
#if INIT_HEADER_DATA_VALIDATE
    if (NUM_HEADER_HANDLERS > (sizeof(header_names) / sizeof(header_names[0])))
    {
        return INIT_HEADER_DATA_ERROR_HEADER_NAMES_SIZE_MISMATCH;
    }
    if (NUM_HEADER_HANDLERS > (sizeof(header_comp_data.header_names_lowercase) / sizeof(header_comp_data.header_names_lowercase[0])))
    {
        return INIT_HEADER_DATA_ERROR_HEADER_NAMES_LC_SIZE_MISMATCH;
    }
#endif

#if INIT_HEADER_DATA_VALIDATE
    size_t min_size = 200, max_size = 0;
    uint64_t sizes = 0;
    uint64_t first_chars = 0;
#endif
    int size_counts[64];
    memset(size_counts, 0, sizeof(size_counts));

    int runs = 2;
    char *pDst = &header_comp_data.header_names_aligned8[0];
    memset(&header_comp_data, 0, sizeof(header_comp_data));

    struct ps sorted[NUM_HEADER_HANDLERS];

    while (runs != 0)
    {
        --runs;

        size_t aligned_size = 0;
        size_t real_size = 0;
        uint64_t i;

        for (i = 0; i < NUM_HEADER_HANDLERS; i++)
        {
            char *pSrc = IMPL_GET_HEADER_NAME_BY_IDX(i);
            size_t s = strlen(pSrc);

#if INIT_HEADER_DATA_VALIDATE
            if (s == 0)
            {
                return 2;
            }

            if (s > HEADER_NAME_MAX_LEN)
            {
                return 20;
            }
#endif
            real_size += (s+1);

            size_t padding = (s & 0x7);

            if (padding != 0)
            {
                padding = 8 - padding;
            }

            if (runs == 1)
            {
                sorted[i].name = pSrc;
                sorted[i].size = s;
                sorted[i].i = i;

#if INIT_HEADER_DATA_VALIDATE
                if (min_size > s)
                {
                    min_size = s;
                }
                if (max_size < s)
                {
                    max_size = s;
                }
#endif

#if INIT_HEADER_DATA_VALIDATE
                if (s < 64)
                {

                    first_chars |= (0x1ul) << ((*pSrc) & 0x3F);
                    sizes |= (0x1ul << (s - 1));
#endif
                    size_t sz = size_counts[s];
#if INIT_HEADER_DATA_VALIDATE
                    if (sz > 5)
                    {
                        return 7;
                    }
#endif

                    size_counts[s] = sz + 1;
                    sz <<= 3;
                    header_comp_data.header_names_values8[s] |= (i << sz);
                    header_comp_data.header_names_offsets8[s] |= ((aligned_size >> 3) << sz);

#if INIT_HEADER_DATA_VALIDATE
                }
                else
                {
                    return 3;
                }
#endif
#if 1
                uint64_t sig = 0;

                while (*pSrc != 0)
                {
                    int shift = (*pSrc) & 0x3F;
                    sig |= (0x1ul) << shift;
                    ++pSrc;
                }

                header_comp_data.header_names_signatures8[s] |= sig;
#endif
                s += padding;
            }
            else
            {
#if 0
                size_t j = 0;
                char* pSrc2 = pSrc;
                for (; j < s; j++)
                {
                    *pDst = *pSrc2;
                    ++pDst;
                    ++pSrc2;
                }
                pDst += padding;
#else
                memcpy(pDst, pSrc, s);
                s += padding;
                pDst += s;
#endif
            }
            aligned_size += s;
        }

        if (runs == 1)
        {
#if INIT_HEADER_DATA_VALIDATE
            if (min_size != HEADER_NAME_MIN_LEN || max_size > HEADER_NAME_MAX_LEN)
            {
                return 4;
            }
            if (sizes != HEADER_SIZE_BITS_CONSTANT)
            {
                // printf("S:%x|%x\n", sizes, HEADER_SIZE_BITS_CONSTANT);
                return 5;
            }
            if (first_chars != HEADER_FIRST_CHAR_BITS_CONSTANT)
            {
                printf("S:%lx | %lx\n", first_chars, HEADER_FIRST_CHAR_BITS_CONSTANT);
                return 51;
            }

            if (aligned_size > sizeof(header_comp_data.header_names_aligned8))
            {
                return 6;
            }
#endif
            printf("//REAL_SIZE: %ld\n", real_size);
        }
    }

    qsort(sorted, NUM_HEADER_HANDLERS, sizeof(struct ps), init_cmp_by_size_and_name);

    uint64_t i;
#if 1
    printf("//SIZES:");
    for (i = 0; i < 64; i++)
    {
        if (size_counts[i] != 0)
        {
            printf("%-2ld ", i);
        }
    }
    printf("\n");

    printf("//COUNT:");
    for (i = 0; i < 64; i++)
    {
        if (size_counts[i] != 0)
        {
            printf("%-2d ", size_counts[i]);
        }
    }
    printf("\n");
#endif
    pDst = &header_comp_data.header_names_aligned8_sorted[0];

    uint64_t *pOffsets = &header_comp_data.header_names_offsets8_sorted[0];
    uint64_t *pCurOffset = pOffsets;
    size_t last_s = 0;
    uint64_t curOffsetValue = 0;

    size_t aligned_size = 0;
    int shift = 0;
    int count = 0;

    for (i = 0; i < NUM_HEADER_HANDLERS; i++)
    {
        char *pSrc = sorted[i].name;

        size_t s = sorted[i].size;
        size_t padding = (s & 0x7);

        if (padding != 0)
        {
            padding = 8 - padding;
        }

        if (last_s != s)
        {
            pCurOffset = pOffsets + s;
            last_s = s;
            curOffsetValue = (aligned_size >> 3);
#if INIT_HEADER_DATA_VALIDATE
            if (curOffsetValue > 0xFF)
            {
                return 101;
            }
#endif
            count = size_counts[s];
            curOffsetValue |= (count << 8);
            shift = 16;
            count = 0;
        }

        count++;
#if INIT_HEADER_DATA_VALIDATE
        if (count > 6)
        {
            return 100;
        }
#endif

        curOffsetValue |= ((uint64_t)sorted[i].i) << shift;
        shift += 8;
        *pCurOffset = curOffsetValue;

        memcpy(pDst, pSrc, s);
        s += padding;
        pDst += s;

        aligned_size += s;
    }
#if 0
    for (i = 0; i < sizeof(header_comp_data.header_names_offsets8_sorted) / sizeof(header_comp_data.header_names_offsets8_sorted[0]); i++)
    {
        printf("OFF:%ld:%lx\n", i, header_comp_data.header_names_offsets8_sorted[i]);
    }
#endif

    return 0;
}
#undef IMPL_GET_HEADER_NAME_BY_IDX

#endif