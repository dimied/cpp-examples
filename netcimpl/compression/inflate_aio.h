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

#define INFLATE_AIO_STATE_BLOCK_DATA 0x8
#define INFLATE_AIO_STATE_INIT_TREE 0x9

#define INFLATE_AIO_STATE_DECODE_TREES_STEP1 10
#define INFLATE_AIO_STATE_DECODE_TREES_STEP2 11
#define INFLATE_AIO_STATE_DECODE_TREES_STEP3 12
#define INFLATE_AIO_STATE_BUILD_TREE 13

#define INFLATE_AIO_STATE_CHECK_BITS 14
#define INFLATE_AIO_STATE_BUILD_FIXED_TREES 15

#define INFLATE_AIO_STATE_BITMASK (0x1F)

#define INFLATE_AIO_STATE_BITSHIFT (5)
#define INFLATE_AIO_STATE_BITSHIFT2 (10)
#define INFLATE_AIO_STATE_BITSHIFT3 (15)

#define INFLATE_AIO_STATE_COMBINE2(S1, S2) (S1 | (S2 << INFLATE_AIO_STATE_BITSHIFT))
#define INFLATE_AIO_STATE_COMBINE3(S1, S2, S3) (S1 | (S2 << INFLATE_AIO_STATE_BITSHIFT) | (S3 << INFLATE_AIO_STATE_BITSHIFT2))
#define INFLATE_AIO_STATE_COMBINE4(S1, S2, S3, S4) (S1 | (S2 << INFLATE_AIO_STATE_BITSHIFT) | (S3 << INFLATE_AIO_STATE_BITSHIFT2) | (S4 << INFLATE_AIO_STATE_BITSHIFT3))

/* encoded offsets and lengths
 *  24:off:256;(144:off:0);(8:off:280);112:off:144
 * #define LAO ((256) | ((144 << 9) | (0x0 << 17)) | ((0x8ul << 26) | (280ul << 34)) | ((112ul << 43) | (144ul << 51)))
 */
#define LAO2 (144 | ((0x8ul << 17) | (280ul << 25)) | ((112ul << 34) | (144ul << 42)))

// static const unsigned char my_clcidx[19] = {
// 16, 17, 18, 0, // 8, 7, 9, 6,
// 10, 5,11, 4,   // 12, 3, 13, 2,
// 14, 1, 15};
#define DDD1 ((8ul << 4) | (7ul << 8) | (9ul << 12) | (6ul << 16))
#define DDD2 ((10ul << 20) | (5ul << 24) | (11ul << 28) | (4ul << 32))
#define DDD3 ((12ul << 36) | (3ul << 40) | (13ul << 44) | (2ul << 48))
/* packing 16*4 bits */
#define DDD (DDD1 | DDD2 | DDD3 | (14ul << 52) | (1ul << 56) | (15ul << 60))
#define LENGTH_OFFSETS_PACKED (DDD >> 4)

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

    INFLATE_ALGO_STEP_PTR(ALGO_STEP_START_IN_PTR, in_data, "");
    INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_START_IN_SIZE, in_size, "");
    INFLATE_ALGO_STEP_PTR(ALGO_STEP_START_OUT_PTR, out_data, "");
    INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_START_OUT_SIZE, out_size, "");

#define STATE_SOURCE state.source
#define STATE_SOURCE_END state.source_end

    unsigned int in_bits, offs, ui32_val;
    unsigned int total_bits;

    unsigned int temp;
    int itemp;

    int out_available = out_size, in_available = in_size;

    int last = 0, error = 0;

    unsigned int comp_state = INFLATE_AIO_STATE_READ_HEADER;

    uint64_t a;
    uint64_t *pUI64;
    const uint64_t *pA;
    struct inflate_tree *lt, *dt, *t;
    // const unsigned char *pCur, *pEnd;

    unsigned int hlit, hdist, hclen;

    unsigned char lengths[288 + 32];

    /* for */
    unsigned char *p_build_tree_len = lengths;
    int build_tree_size = 0;

    unsigned int i, num;

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
        /* Extract state to process. */
        unsigned int cur_state = comp_state & INFLATE_AIO_STATE_BITMASK;

        comp_state >>= INFLATE_AIO_STATE_BITSHIFT;

        if (cur_state < 4)
        {
            INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_CHANGE_TO, cur_state, "");
        }

        switch (cur_state)
        {
        case INFLATE_AIO_STATE_HEADER_RAW:
        {
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_RAW);
            INFLATE_ALGO_STEP(ALGO_STEP_RAW);
            // read and process
            // comp_state = INFLATE_AIO_STATE_READ_UINT32 | (INFLATE_AIO_STATE_HEADER_RAW_STEP1 << 8) | (INFLATE_AIO_STATE_NEW_STATE << 16);
            in_available = (state.source_end - state.source);
            if (in_available < 4)
            {
                error = INFLATE_ERROR_DATA;
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "avail <4");
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
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "RAW:size-check");
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
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "RAW:overflow");
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
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "NO-ERROR");
                    break;
                }

                comp_state = INFLATE_AIO_STATE_NEW_STATE;
            }
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_HEADER_FIXED:
        {
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_FIXED);
            INFLATE_ALGO_STEP(ALGO_STEP_FIXED_TREE);
            lt = &fixed.length_tree;
            dt = &fixed.distance_tree;
            if (fixed_done == 0)
            {
                comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_STATE_BUILD_FIXED_TREES, INFLATE_AIO_STATE_BLOCK_DATA, INFLATE_AIO_STATE_NEW_STATE);
                break;
            }
            comp_state = INFLATE_AIO_STATE_COMBINE2(INFLATE_AIO_STATE_BLOCK_DATA, INFLATE_AIO_STATE_NEW_STATE);
        }
        break;
            /* -- */
        case INFLATE_AIO_STATE_HEADER_DYN:
        {
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_DYN);
            INFLATE_ALGO_STEP(ALGO_STEP_DYN_TREE);
            lt = &dynamic.length_tree;
            lt->max_sym = -1;
            dt = &dynamic.distance_tree;
            dt->max_sym = -1;

            /* read 14 bits */
            in_bits = 14;
            offs = 0;
            INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_GET_BITS, 14, "HEADER-DYN");
            comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_STATE_GET_BITS, INFLATE_AIO_STATE_DECODE_TREES_STEP1, INFLATE_AIO_STATE_DECODE_TREES_STEP2);
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_HEADER_ERROR:
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_ERROR);
            error = 100;
            break;
        /* -- */
        case INFLATE_AIO_STATE_DECODE_TREES_STEP1:
        {
            /* 5 bits for HLIT (257-286) */
            hlit = ((unsigned int)(in_bits & 0x1F)) + 257;
            in_bits >>= 5;
            /* 5 bits for HDIST (1-32) */
            hdist = ((unsigned int)(in_bits & 0x1F)) + 1;
            in_bits >>= 5;
            /* 4 bits for HCLEN (4-19) */
            hclen = in_bits + 4;

            INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, hlit, "hlit");
            INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, hdist, "hdist");
            INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, hclen, "hclen");

            inflate_nullify(&dynamic, sizeof(dynamic));

            /*
             * The RFC limits the range of HLIT to 286.
             * HDIST has range 1-32.
             * Distance codes 30 and 31 have no meaning.
             * We treat them as error here.
             *
             * zlib does it also: https://github.com/madler/zlib/issues/82
             */
            if (hlit > 286 || hdist > 30)
            {
                error = INFLATE_ERROR_DATA;
                INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_ERROR, error, "(hlit > 286 || hdist > 30)");
                break;
            }
#if 1
            memset(lengths, 0, sizeof(lengths));
#else
            pUI64 = (ui64bit_t *)lengths;
            *pUI64 = 0;
            pUI64++;
            *pUI64 = 0;
            pUI64++;
            *pUI64 = 0;
#endif
            total_bits = hclen * 3;
            in_bits = 12;
            offs = 0;
            comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_STATE_CHECK_BITS, INFLATE_AIO_STATE_GET_BITS, INFLATE_AIO_STATE_DECODE_TREES_STEP2);
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_DECODE_TREES_STEP2:
        {
            /* We don't use any lookup array/table and compute the lengths */
            temp = state.bitbuf;
            itemp = state.bitcount;

            for (i = 0; i < 3; ++i)
            {
                lengths[i + 16] = (unsigned char)(temp & 0x7);
                temp >>= 3;
            }
            lengths[0] = (unsigned char)(temp & 0x7);
            temp >>= 3;
            itemp -= 12;
            i++;

            uint64_t offset_container = LENGTH_OFFSETS_PACKED;

            const unsigned char *pSrc = state.source;

            for (; i < hclen; ++i)
            {
                if (itemp < 3)
                {
                    temp |= ((unsigned int)*pSrc) << itemp;
                    pSrc++;
                    itemp += 8;
                }

                const unsigned int k = (unsigned int)(offset_container & 0xF);
                offset_container >>= 4;

                /* 3 bits for code length (0-7) */
                lengths[k] = (unsigned char)(temp & 0x7);
                temp >>= 3;
                itemp -= 3;
            }
            state.source = pSrc;
            state.bitbuf = temp;
            state.bitcount = itemp;

            /* Build code length tree (in literal/length tree to save space) */
            // error = inflate_build_tree(lt, lengths, 19);
            t = lt;
            p_build_tree_len = lengths;
            build_tree_size = 19;

            comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_STATE_BUILD_TREE, INFLATE_AIO_STATE_DECODE_TREES_STEP3, INFLATE_AIO_STATE_BLOCK_DATA);
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_BUILD_TREE:
        {
            error = inflate_build_tree(t, p_build_tree_len, build_tree_size);
            /* Check code length tree is not empty */
#if INFLATE_DO_CHECK_FOR_ERRORS
            if (t->max_sym == -1)
            {
                TEST_PRINTF("ERROR: max sym: %d\n", lt->max_sym);
                error = INFLATE_ERROR_DATA;
            }
#endif
        }
        break;
            /* -- */
        case INFLATE_AIO_STATE_DECODE_TREES_STEP3:
        {
            // int sym;
            /* Decode code lengths for the dynamic trees */
            for (num = 0; num < hlit + hdist;)
            {
                // int sym = inflate_decode_symbol(d, lt);
                ////
                int sym, base = 0, offset = 0, len;

                /*
                 * Get more bits while code index is above number of codes
                 *
                 * Rather than the actual code, we are computing the position of the
                 * code in the sorted order of codes, which is the index of the
                 * corresponding symbol.
                 *
                 * Conceptually, for each code length (level in the tree), there are
                 * counts[len] leaves on the left and internal nodes on the right.
                 * The index we have decoded so far is base + offs, and if that
                 * falls within the leaves we are done. Otherwise we adjust the range
                 * of offs and add one more bit to it.
                 */
                itemp = state.bitcount;
                temp = state.bitbuf;

                for (len = 1;; ++len)
                {
#if INFLATE_DO_CHECK_FOR_ERRORS
                    if (len > 15)
                    {
                        printf("if (len > 15)\n");
                        error = INFLATE_ERROR_DATA;
                        break;
                    }
#endif
                    if (itemp == 0)
                    {
#if INFLATE_DO_CHECK_FOR_ERRORS
                        if (state.source == state.source_end)
                        {
                            error = INFLATE_ERROR_BUFFER;
                            break;
                        }
#endif
                        temp |= (unsigned int)*state.source;
                        state.source++;
                        itemp = 8;
                    }
                    offset = (offset << 1) | (temp & 0x1);
                    temp >>= 1;
                    itemp--;
                    const int c = lt->code_counts.us16[len];

                    if (offset < c)
                    {
                        break;
                    }

                    base += c;
                    offset -= c;
                }
                state.bitcount = itemp;
                state.bitbuf = temp;
#if INFLATE_DO_CHECK_FOR_ERRORS
                if (error != 0)
                {
                    TEST_PRINTF("error=%d\n", error);
                    error = INFLATE_ERROR_DATA;
                    break;
                }
#endif

                base += offset;
                // assert(base + offs >= 0 && base + offs < 288);
#if INFLATE_DO_CHECK_FOR_ERRORS
                if (base < 0 || base > 287)
                {
                    printf("if (base < 0 || base > 287)\n");
                    error = INFLATE_ERROR_DATA;
                    break;
                }
#endif

                sym = lt->symbols.us16[base];
                ////

#if INFLATE_DO_CHECK_FOR_ERRORS
                if (error != 0 || sym > lt->max_sym)
                {
                    // printf("ERROR:%d: sym > max sym | %d > %d\n", error, sym, lt->max_sym);
                    error = INFLATE_ERROR_DATA;
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "STEP3:DECODE-SYMBOL");
                    break;
                }
#endif

                int length = 1;
                // printf("SYM-IN: %d (num=%d)\n", sym, num);

                switch (sym)
                {
                case 16:
                    /* Copy previous code length 3-6 times (read 2 bits) */
#if INFLATE_DO_CHECK_FOR_ERRORS
                    if (num == 0)
                    {
                        printf("NUM=0\n");
                        error = INFLATE_ERROR_DATA;
                    }
                    else
                    {
                        sym = lengths[num - 1];
                    }
#else
                    sym = lengths[num - 1];
#endif

                    // length = inflate_getbits(d, 2, 3);
                    length = ((2 << 4) | (3));
                    break;
                case 17:
                    /* Repeat code length 0 for 3-10 times (read 3 bits) */
                    sym = 0;
                    // length = inflate_getbits(d, 3, 3);
                    length = ((3 << 4) | (3));
                    break;
                case 18:
                    /* Repeat code length 0 for 11-138 times (read 7 bits) */
                    sym = 0;
                    // length = inflate_getbits(d, 7, 11);
                    length = ((7 << 4) | (11));
                    break;
                default:
                    /* Values 0-15 represent the actual code lengths */
                    // length = 1;
                    break;
                }
                if (error != 0)
                {
                    break;
                }
                if (length != 1)
                {
                    offset = length & 0xF;
                    length >>= 4;

                    if (length < itemp)
                    {
#if INFLATE_DO_CHECK_FOR_ERRORS
                        if (state.source == state.source_end)
                        {
                            printf("EOID!\n");
                            error = INFLATE_ERROR_BUFFER;
                            break;
                        }
#endif
                        temp |= (unsigned int)*state.source << itemp;
                        ++state.source;
                        itemp += 8;
                    }
                    in_bits = (0x1 << length) - 1;
                    in_bits = (temp & in_bits) + offset;
                    itemp -= length;
                    temp >>= length;
                    length = in_bits;
                }
                // TEST_PRINTF("SYM-OUT: %d (len=%d)\n", sym, length);

                state.bitcount = itemp;
                state.bitbuf = temp;

#if INFLATE_DO_CHECK_FOR_ERRORS
                if ((unsigned)length > hlit + hdist - num)
                {
                    error = INFLATE_ERROR_DATA;
                    break;
                }
#endif

                while (length-- != 0)
                {
                    lengths[num++] = sym;
                }
            }
            if (error != 0)
            {
                break;
            }

            /* Is end-of-block symbol present? */
#if INFLATE_DO_CHECK_FOR_ERRORS
            if (lengths[256] == 0)
            {
                printf("if (lengths[256] == 0)\n");
                error = INFLATE_ERROR_DATA;
                break;
            }
#endif

            /* Build dynamic trees */
            error = inflate_build_tree(lt, lengths, hlit);

#if INFLATE_DO_CHECK_FOR_ERRORS
            if (error != INFLATE_OK)
            {
                TEST_PRINTF("ERROR: from build tree:hlit %d\n", error);
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "STEP3:BT");
                // return error;
                break;
            }
#endif

            error = inflate_build_tree(dt, lengths + hlit, hdist);
        }
            comp_state = INFLATE_AIO_STATE_COMBINE2(INFLATE_AIO_STATE_BLOCK_DATA, INFLATE_AIO_STATE_NEW_STATE);
            break;
            /* -- */
        case INFLATE_AIO_STATE_BLOCK_DATA:
            /* Decode block using decoded trees */
            error = inflate_block_data(&state, lt, dt);
            break;
            /* -- */
        case INFLATE_AIO_STATE_NEW_STATE:
            // read new block?
            // if last was set, then we already computed the last block.
            error = last;
            comp_state = INFLATE_AIO_STATE_READ_HEADER;
            break;
        /* -- */
        case INFLATE_AIO_STATE_CHECK_BITS:
        {
#if INFLATE_DO_CHECK_FOR_ERRORS
            struct inflate_state *d = &state;

            itemp = state.bitcount;

            if (total_bits > (unsigned int)itemp)
            {
                if (d->source >= d->source_end)
                {
                    error = INFLATE_ERROR_DATA;
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "CHECK-BITS:overflow");
                    break;
                }
                itemp += ((d->source_end - d->source) << 3);

                if (total_bits > (unsigned int)itemp)
                {
                    error = INFLATE_ERROR_DATA;
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "total_bits");
                }
            }
#endif
        }
        break;
        case INFLATE_AIO_STATE_GET_BITS:
        {
            // INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_GET_BITS, in_bits);
            itemp = state.bitcount;

#if INFLATE_DO_CHECK_FOR_ERRORS
            if (itemp < 0)
            {
                error = INFLATE_ERROR_BUFFER;
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "bc<0");
                break;
            }

            // We read no more than 24 bits at once.
            if (in_bits > 24)
            {
                state.bitcount = -1;
                error = INFLATE_ERROR_DATA;
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "in_bits>24");
                break;
            }
#endif
            temp = state.bitbuf;

            register const unsigned char *pCur2 = state.source;

#if INFLATE_DO_CHECK_FOR_ERRORS
            register const unsigned char *pEnd2 = state.source_end;
#endif

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
                temp |= (unsigned int)*pCur2 << itemp;
                ++pCur2;
#endif
                itemp += 8;
            }
            state.source = pCur2;

#if INFLATE_DO_CHECK_FOR_ERRORS
            if (itemp == 0 || itemp > 30 || in_bits > (unsigned int)itemp)
            {
                state.bitcount = -1;
                error = INFLATE_ERROR_BUFFER;
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "GET-BITS");
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
        /* -- */
        case INFLATE_AIO_STATE_READ_HEADER:
        {
            INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_GET_BITS, 3, "READ-HEADER");
            // read 3, last one will be flag for last block
            in_bits = 3;
            offs = 0;
            comp_state = INFLATE_AIO_STATE_COMBINE2(INFLATE_AIO_STATE_GET_BITS, INFLATE_AIO_STATE_READ_HEADER_AFTER);
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_READ_HEADER_AFTER:
        {
            /* BTYPE specifies how the data are compressed, as follows:
             * 00 - no compression, 01 - compressed with fixed Huffman codes,
             * 10 - compressed with dynamic Huffman codes, 11 - reserved (error)
             */
            INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, in_bits, "");
            comp_state = in_bits;
            last = comp_state & 0x1;
            comp_state >>= 1;
            //INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_CHANGE_TO, comp_state, "");
            if (comp_state > 2)
            {
                error = 4;
            }
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_BUILD_FIXED_TREES:
        {
            if (fixed_done == 0)
            {
                INFLATE_ALGO_STEP(ALGO_STEP_BUILD_FIXED_TREE);

                inflate_nullify(&fixed, sizeof(fixed));

                lt->max_sym = 285;

                lt->code_counts.us16[7] = 24;
                lt->code_counts.us16[8] = 152;
                lt->code_counts.us16[9] = 112;

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
        }
        break;
            /* -- */
        default:
            // Some invalid internal algorithm state!
            error = 1000;
            break;
        }
    } while (error == 0);

    if (error == 1)
    {
        --error;
    }

    return error;
}

#undef LAO2

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