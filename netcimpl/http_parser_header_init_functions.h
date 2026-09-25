#ifndef HTTP_PARSER_HEADER_INIT_FUNCTIONS
#define HTTP_PARSER_HEADER_INIT_FUNCTIONS

#define IMPL_GET_HEADER_NAME_BY_IDX(IDX) (headerHandlers[IDX].name)

#ifndef INIT_HEADER_DATA_VALIDATE
#define INIT_HEADER_DATA_VALIDATE 1
#endif

int init_header_data()
{
#if INIT_HEADER_DATA_VALIDATE
    if (NUM_HEADER_HANDLERS != (sizeof(headerHandlers) / sizeof(headerHandlers[0])))
    {
        return 1;
    }
#endif

#if INIT_HEADER_DATA_VALIDATE
    size_t min_size = 200, max_size = 0;
    uint64_t sizes = 0;
#endif
    int size_counts[64];
    memset(size_counts, 0, sizeof(size_counts));

    int runs = 2;
    char *pDst = &header_names_aligned8[0];

    while (runs != 0)
    {
        --runs;
        uint64_t i;
        size_t aligned_size = 0;

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

            size_t padding = (s & 0x7);

            if (padding != 0)
            {
                padding = 8 - padding;
            }

            if (runs == 1)
            {
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

                    sizes |= (0x1ul << (s - 1));
#endif
                    size_t sz = size_counts[s];
#if INIT_HEADER_DATA_VALIDATE
                    if (sz > 7)
                    {
                        return 7;
                    }
#endif

                    size_counts[s] = sz + 1;
                    sz <<= 3;
                    header_names_values8[s] |= (i << sz);
                    header_names_offsets8[s] |= ((aligned_size >> 3) << sz);

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

                header_names_signatures8[s] |= sig;
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

            if (aligned_size > sizeof(header_names_aligned8))
            {
                return 6;
            }
#endif
            memset(header_names_aligned8, 0, sizeof(header_names_aligned8));
        }
    }

    return 0;
}
#undef IMPL_GET_HEADER_NAME_BY_IDX

#endif