#ifndef INFLATE_ALL_IN_ONE_INCLUDED
#define INFLATE_ALL_IN_ONE_INCLUDED

#ifdef INFLATE_AIO_STATE_HEADER_RAW
#error "INFLATE_AIO_STATE_HEADER_RAW predefined"
#endif

#ifdef INFLATE_AIO_STATE_HEADER_FIXED
#error "INFLATE_AIO_STATE_HEADER_FIXED predefined"
#endif

#ifdef INFLATE_AIO_STATE_HEADER_DYN
#error "INFLATE_AIO_STATE_HEADER_DYN predefined"
#endif

#ifdef INFLATE_AIO_STATE_HEADER_ERROR
#error "INFLATE_AIO_STATE_HEADER_ERROR predefined"
#endif

#ifdef INFLATE_AIO_STATE_GET_BITS
#error "INFLATE_AIO_STATE_GET_BITS predefined"
#endif

#ifdef INFLATE_AIO_STATE_NEW_STATE
#error "INFLATE_AIO_STATE_NEW_STATE predefined"
#endif

#ifdef INFLATE_AIO_STATE_READ_HEADER
#error "INFLATE_AIO_STATE_READ_HEADER predefined"
#endif

#ifdef INFLATE_AIO_STATE_READ_HEADER_AFTER
#error "INFLATE_AIO_STATE_READ_HEADER_AFTER predefined"
#endif

/* These are some states for all-in-one function */

#define INFLATE_AIO_STATE_HEADER_RAW 0x0
#define INFLATE_AIO_STATE_HEADER_FIXED 0x1
#define INFLATE_AIO_STATE_HEADER_DYN 0x2
#define INFLATE_AIO_STATE_HEADER_ERROR 0x3

#define INFLATE_AIO_STATE_GET_BITS 0x4
#define INFLATE_AIO_STATE_NEW_STATE 0x5

#define INFLATE_AIO_STATE_READ_HEADER 0x6
#define INFLATE_AIO_STATE_READ_HEADER_AFTER 0x7

// #define INFLATE_AIO_STATE_READ_UINT32 0x10
// #define INFLATE_AIO_STATE_HEADER_RAW_STEP1 0x11

#define INFLATE_AIO_STATE_DECODE_TREES 0x8
#define INFLATE_AIO_STATE_BLOCK_DATA 0x9

#define INFLATE_AIO_STATE_BITMASK (0xFF)

#define INFLATE_AIO_STATE_BITSHIFT (8)
#define INFLATE_AIO_STATE_BITSHIFT2 (16)

#define INFLATE_AIO_STATE_COMBINE2(S1, S2) (S1 | (S2 << INFLATE_AIO_STATE_BITSHIFT))
#define INFLATE_AIO_STATE_COMBINE3(S1, S2, S3) (S1 | (S2 << INFLATE_AIO_STATE_BITSHIFT) | (S3 << INFLATE_AIO_STATE_BITSHIFT2))

static void inflate_nullify(void *p, size_t size)
{
    memset(p, 0, size);
}
/**
 *
 */
int inflate_aio(unsigned char *out_data, unsigned int out_size, unsigned char *in_data, unsigned int in_size)
{
    /* to make it a single function we use some state variable and iterate */
#if GLOBAL_DATA_STRUCTS == 0

    struct inflate_tree_pair dynamic;

    struct inflate_data state;
#endif

#if 0
    unsigned int lengths[16];
    unsigned int offsets[16];
#endif
    inflate_nullify(&state, sizeof(state));

    state.source = (const unsigned char *)in_data;
    state.source_end = state.source + in_size;

    state.destination = (unsigned char *)out_data;
    state.destination_start = state.destination;
    state.destination_end = state.destination + out_size;

#define STATE_SOURCE state.source
#define STATE_SOURCE_END state.source_end

    printf("SRC: %p\n", STATE_SOURCE);

    unsigned int in_bits, offs, ui32_val;

    unsigned int temp;
    int itemp;

    int out_available = out_size, in_available = in_size;

    struct inflate_tree *lt, *dt;

    int last = 0, error = 0;

    unsigned int comp_state = INFLATE_AIO_STATE_READ_HEADER;

    uint64_t a;
    uint64_t *pUI64;
    const uint64_t *pA;
    // const unsigned char *pCur, *pEnd;

    do
    {
#if INFLATE_DO_CHECK_FOR_ERRORS
        if (out_available <= 0 || in_available < 0)
        {
            error = INFLATE_ERROR_DATA;
            break;
        }

        if (in_available == 0 && comp_state != INFLATE_AIO_STATE_NEW_STATE)
        {
            error = INFLATE_ERROR_DATA;
        }
#endif
        unsigned int cur_state = comp_state & INFLATE_AIO_STATE_BITMASK;

        comp_state >>= INFLATE_AIO_STATE_BITSHIFT;

        switch (cur_state)
        {
        case INFLATE_AIO_STATE_HEADER_RAW:
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_RAW);
            // read and process
            // comp_state = INFLATE_AIO_STATE_READ_UINT32 | (INFLATE_AIO_STATE_HEADER_RAW_STEP1 << 8) | (INFLATE_AIO_STATE_NEW_STATE << 16);
            in_available = (state.source_end - state.source);
            if (in_available < 4)
            {
                error = INFLATE_ERROR_DATA;
            }
            else
            {
                ui32_val = *(unsigned int *)state.source;
                state.source += 4;
                in_available -= 4;

#if INFLATE_DO_CHECK_FOR_ERRORS
                temp = ((unsigned int)(ui32_val >> 16));
                temp = (~temp) & 0xFFFF;
                ui32_val &= 0xFFFF;

                if (temp != ui32_val)
                {
                    error = INFLATE_ERROR_DATA;
                    break;
                }
#else
                ui32_val &= 0xFFFF;
#endif

#if DO_OVERFLOW_TEST
                // Overflows?
                if ((int)ui32_val > out_available || (int)ui32_val > in_available)
                {
                    error = INFLATE_ERROR_DATA;
                    break;
                }
#endif
                memcpy(state.destination, state.source, (size_t)ui32_val);
                out_available -= ui32_val;
                in_available -= ui32_val;

                state.source += ui32_val;
                state.destination += ui32_val;

                state.bitbuf = 0;
                state.bitcount = 0;

                if (in_available == 0)
                {
                    error = 1;
                    break;
                }

                comp_state = INFLATE_AIO_STATE_NEW_STATE;
            }

            break;
#if 0
        case INFLATE_AIO_STATE_HEADER_RAW_STEP1:

#if INFLATE_DO_CHECK_FOR_ERRORS
            temp = ((unsigned int)(ui32_val >> 16));
            temp = (~temp) & 0xFFFF;
            ui32_val &= 0xFFFF;

            if (temp != ui32_val)
            {
                error = INFLATE_ERROR_DATA;
                break;
            }
#else
            ui32_val &= 0xFFFF;
#endif

#if DO_OVERFLOW_TEST
            // Overflows?
            if ((int)ui32_val > out_available || (int)ui32_val > in_available)
            {
                error = INFLATE_ERROR_DATA;
                break;
            }
#endif
            memcpy(state.destination, state.source, (size_t)ui32_val);
            out_available -= ui32_val;
            in_available -= ui32_val;

            if (in_available == 0)
            {
                error = 1;
                break;
            }
            state.source += ui32_val;
            state.destination += ui32_val;
            state.bitbuf = 0;
            state.bitcount = 0;
            break;
#endif
        case INFLATE_AIO_STATE_HEADER_FIXED:
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_FIXED);
            lt = &fixed.length_tree;
            dt = &fixed.distance_tree;

            if (fixed_done == 0)
            {
                inflate_nullify(&fixed, sizeof(fixed));

                lt->max_sym = 285;

#if 0
  unsigned short *pC = &lt->code_counts[7];
  unsigned int v2 = (24|(152<<8)|(112<<16));

  for(i=0; i < 3; i++) {
    *pC = (v2&0xFF);
    v2>>=8;
    ++pC;
  }
#else

                lt->code_counts.us16[7] = 24;
                lt->code_counts.us16[8] = 152;
                lt->code_counts.us16[9] = 112;
#endif

// encoded offsets and lengths
//  24:off:256;(144:off:0);(8:off:280);112:off:144
#define LAO ((256) | ((144 << 9) | (0x0 << 17)) | ((0x8ul << 26) | (280ul << 34)) | ((112ul << 43) | (144ul << 51)))
#define LAO2 (144 | ((0x8ul << 17) | (280ul << 25)) | ((112ul << 34) | (144ul << 42)))

#if INFLATE_BUILD_FIXED_TREES_ALGO == 1
                unsigned short *pS = t->symbols.us16;
                int i;
                uint64_t v = LAO2;
                int l = 24, offset = 256;
                do
                {
                    for (i = 0; i < l; ++i, ++pS, ++offset)
                    {
                        *pS = (unsigned short)(offset);
                    }

                    l = (int)(v & 0x1FFFF);
                    offset = (l >> 8) & 0xFF;
                    l &= 0xFF;
                    v >>= 17;
                } while (l != 0);

#elif INFLATE_BUILD_FIXED_TREES_ALGO == 2
                unsigned short *pS = t->symbols.us16;
                pS = inflate_set_inc(pS, 24, 256);
                pS = inflate_set_inc(pS, 144, 0);
                pS = inflate_set_inc(pS, 8, 280);
                inflate_set_inc(pS, 112, 144);

#elif INFLATE_BUILD_FIXED_TREES_ALGO == 3
                pUI64 = lt->symbols.ui64;
                pA = inflate_adds;

                temp = (6 | (36 << 8) | (2 << 16) | (28 << 24));

                while (temp != 0)
                {
                    a = *pA;
                    error = (int)(temp & 0xFF);

                    for (; error != 0; error--)
                    {
                        *pUI64 = a;
                        a += ADD1;
                        ++pUI64;
                    }
                    ++pA;
                    temp >>= 8;
                }
#else
                unsigned short *pS = t->symbols.us16;
                int i;
                for (i = 0; i < 24; ++i)
                {
                    pS[i] = 256 + i;
                }
                for (i = 0; i < 144; ++i)
                {
                    pS[24 + i] = i;
                }
                for (i = 0; i < 8; ++i)
                {
                    pS[24 + 144 + i] = 280 + i;
                }
                for (i = 0; i < 112; ++i)
                {
                    pS[24 + 144 + 8 + i] = 144 + i;
                }
#endif
                dt->max_sym = 29;
                /* Build fixed distance tree */
                dt->code_counts.us16[5] = 32;

#if INFLATE_BUILD_FIXED_TREES_ALGO == 2
                inflate_set_inc(dt->symbols.us16, 32, 0);

#elif INFLATE_BUILD_FIXED_TREES_ALGO == 3
                pUI64 = dt->symbols.ui64;
                a = UI64_REPEAT_INC(0UL);
                error = 8;

                for (; error > 0; error--)
                {
                    *pUI64 = a;
                    a += ADD1;
                    ++pUI64;
                }
#else
                for (i = 0; i < 32; ++i)
                {
                    // printf("INC-SET: %d\n", i);
                    t->symbols.us16[i] = i;
                }
#endif

                ++fixed_done;
            }

            comp_state = INFLATE_AIO_STATE_COMBINE2(INFLATE_AIO_STATE_BLOCK_DATA, INFLATE_AIO_STATE_NEW_STATE);
            break;
        case INFLATE_AIO_STATE_HEADER_DYN:
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_DYN);
            inflate_nullify(&dynamic, sizeof(dynamic));
            lt = &dynamic.length_tree;
            lt->max_sym = -1;
            dt = &dynamic.distance_tree;
            dt->max_sym = -1;

            comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_STATE_DECODE_TREES, INFLATE_AIO_STATE_BLOCK_DATA, INFLATE_AIO_STATE_NEW_STATE);
            break;
        case INFLATE_AIO_STATE_HEADER_ERROR:
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_ERROR);
            error = 100;
            break;
        case INFLATE_AIO_STATE_DECODE_TREES:
            error = inflate_decode_trees(&state, lt, dt);
            break;
        case INFLATE_AIO_STATE_BLOCK_DATA:
            /* Decode block using decoded trees */
            error = inflate_block_data(&state, lt, dt);
            break;
        case INFLATE_AIO_STATE_NEW_STATE:
            // read new block?
            // if last was set, then we already computed the last block.
            error = last;
            comp_state = INFLATE_AIO_STATE_READ_HEADER;
            break;
#if 0
        case INFLATE_AIO_STATE_READ_UINT32:
            in_available = (state.source_end - state.source);
            if (in_available < 4)
            {
                error = INFLATE_ERROR_DATA;
            }
            else
            {
                ui32_val = *(unsigned int *)state.source;
                state.source += 4;
                in_available -= 4;
            }
            break;
#endif
            //
            //
        case INFLATE_AIO_STATE_GET_BITS:
        {
            itemp = state.bitcount;

#if INFLATE_DO_CHECK_FOR_ERRORS
            if (itemp < 0)
            {
                error = INFLATE_ERROR_BUFFER;
                break;
            }

            // We read no more than 24 bits at once.
            if (in_bits > 24)
            {
                state.bitcount = -1;
                error = INFLATE_ERROR_DATA;
                break;
            }
#endif
            temp = state.bitbuf;

            register const unsigned char *pEnd2 = state.source_end;
            register const unsigned char *pCur2 = state.source;

            while (itemp < (int)in_bits)
            {
#if INFLATE_DO_CHECK_FOR_ERRORS
                if (pCur2 < pEnd2)
                {
                    temp |= (unsigned int)*pCur2 << itemp;
                    ++pCur2;
                }
                else
                {
                    itemp = 0;
                    break;
                }
#else
                temp |= (unsigned int)*pCur << itemp;
                ++pCur;
#endif
                itemp += 8;
            }
            state.source = pCur2;

#if INFLATE_DO_CHECK_FOR_ERRORS
            if (itemp == 0 || itemp > 30 || in_bits > (unsigned int)itemp)
            {
                state.bitcount = -1;
                error = INFLATE_ERROR_BUFFER;
                break;
            }

#endif
            /* minus requested bits */
            itemp -= in_bits;
            state.bitcount = itemp;

            /* Bits from bit buffer! */
            itemp = temp & ((0x1UL << in_bits) - 1);

            /* store for future requests */
            temp >>= in_bits;
            state.bitbuf = temp;

            in_available = (STATE_SOURCE_END - STATE_SOURCE);

            in_bits = (itemp + offs);
        }
        break;
        case INFLATE_AIO_STATE_READ_HEADER:
            // read 3, last one will be flag for last block
            in_bits = 3;
            offs = 0;
            comp_state = INFLATE_AIO_STATE_COMBINE2(INFLATE_AIO_STATE_GET_BITS, INFLATE_AIO_STATE_READ_HEADER_AFTER);
            break;
        case INFLATE_AIO_STATE_READ_HEADER_AFTER:
            // BTYPE specifies how the data are compressed, as follows:
            // 00 - no compression, 01 - compressed with fixed Huffman codes,
            // 10 - compressed with dynamic Huffman codes, 11 - reserved (error)
            comp_state = in_bits;
            last = comp_state & 0x1;
            comp_state >>= 1;
            break;
        default:
            // Some invalid internal algorithm state!
            return 1000;
        }
    } while (error == 0);

    if (error == 1)
    {
        --error;
    }

    return error;
}

#undef INFLATE_AIO_STATE_HEADER_RAW
#undef INFLATE_AIO_STATE_HEADER_FIXED
#undef INFLATE_AIO_STATE_HEADER_DYN
#undef INFLATE_AIO_STATE_HEADER_ERROR

#undef INFLATE_AIO_STATE_GET_BITS
#undef INFLATE_AIO_STATE_NEW_STATE

#undef INFLATE_AIO_STATE_READ_HEADER
#undef INFLATE_AIO_STATE_READ_HEADER_AFTER

#undef INFLATE_AIO_STATE_READ_UINT32
#undef INFLATE_AIO_STATE_HEADER_RAW_STEP1

#undef INFLATE_AIO_STATE_DECODE_TREES
#undef INFLATE_AIO_STATE_BLOCK_DATA

#undef INFLATE_AIO_STATE_INIT_FIXED_TREE

#endif