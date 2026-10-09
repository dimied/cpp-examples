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
#define INFLATE_AIO_STATE_INIT_TREE 0x6

#define INFLATE_AIO_STATE_READ_HEADER 0x8
#define INFLATE_AIO_STATE_READ_HEADER_AFTER 0x9
// #define INFLATE_AIO_STATE_INIT_TREE 10
#define INFLATE_AIO_STATE_BUILD_TREE 11

#define INFLATE_AIO_STATE_DECODE_TREES_STEP1 12
#define INFLATE_AIO_STATE_DECODE_TREES_STEP2 13
#define INFLATE_AIO_STATE_DECODE_TREES_STEP3 14
#define INFLATE_AIO_STATE_DECODE_TREES_STEP3_SYM 15

#define INFLATE_AIO_DECODE_SYMBOL 19
#define INFLATE_AIO_COPY_DATA 20
#define INFLATE_AIO_STATE_BLOCK_DATA 21
#define INFLATE_AIO_STATE_BLOCK_DATA_STEP2 22
#define INFLATE_AIO_STATE_BLOCK_DATA_STEP3 23
#define INFLATE_AIO_STATE_BUILD_TREE_DT 24

#define INFLATE_AIO_STATE_BUILD_FIXED_TREES 25

#define INFLATE_AIO_STATE_BITMASK (0x1F)

#define INFLATE_AIO_STATE_BITSHIFT (5)
#define INFLATE_AIO_STATE_BITSHIFT2 (10)
#define INFLATE_AIO_STATE_BITSHIFT3 (15)

#define INFLATE_AIO_STATE_COMBINE2(S1, S2) (S1 | (S2 << INFLATE_AIO_STATE_BITSHIFT))
#define INFLATE_AIO_STATE_COMBINE3(S1, S2, S3) (S1 | (S2 << INFLATE_AIO_STATE_BITSHIFT) | (S3 << INFLATE_AIO_STATE_BITSHIFT2))
#define INFLATE_AIO_STATE_COMBINE4(S1, S2, S3, S4) (S1 | (S2 << INFLATE_AIO_STATE_BITSHIFT) | (S3 << INFLATE_AIO_STATE_BITSHIFT2) | (S4 << INFLATE_AIO_STATE_BITSHIFT3))

#ifndef INFLATE_AIO_ALLOW_NO_DESTINATION
#define INFLATE_AIO_ALLOW_NO_DESTINATION 0
#endif

/* encoded offsets and lengths
 *  24:off:256;(144:off:0);(8:off:280);112:off:144
 * #define LAO ((256) | ((144 << 9) | (0x0 << 17)) | ((0x8ul << 26) | (280ul << 34)) | ((112ul << 43) | (144ul << 51)))
 */
#define LAO2 (144 | ((0x8ul << 17) | (280ul << 25)) | ((112ul << 34) | (144ul << 42)))

// static const unsigned char my_clcidx[19] = {
// 16, 17, 18, 0, // 8, 7, 9, 6,
// 10, 5,11, 4,   // 12, 3, 13, 2,
// 14, 1, 15};
#define INFLATE_AIO_LENGTH_OFFSETS_1 ((8ul) | (7ul << 4) | (9ul << 8) | (6ul << 12))
#define INFLATE_AIO_LENGTH_OFFSETS_2 ((10ul << 16) | (5ul << 20) | (11ul << 24) | (4ul << 28))
#define INFLATE_AIO_LENGTH_OFFSETS_3 ((12ul << 32) | (3ul << 36) | (13ul << 40) | (2ul << 44))
#define INFLATE_AIO_LENGTH_OFFSETS_4 ((14ul << 48) | (1ul << 52) | (15ul << 56))
/* packing 15*4 bits,
 * starting with 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
 */
#define INFLATE_AIO_LENGTH_OFFSETS_PACKED (INFLATE_AIO_LENGTH_OFFSETS_1 | INFLATE_AIO_LENGTH_OFFSETS_2 | INFLATE_AIO_LENGTH_OFFSETS_3 | INFLATE_AIO_LENGTH_OFFSETS_4)

static inflate_tree_pair dynamic;

unsigned char lengths[288 + 32];

union
{
    uint64_t ui64[4];
    unsigned short us16[16];
} offsets;

#define INFLATE_AIO_STEP_DESC(C, N) {C, N}

struct inflate_step_name
{
    int step;
    const char *name;
};

struct inflate_step_name step_names[30] = {
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_HEADER_RAW, "RAW"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_HEADER_FIXED, "FIXED"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_HEADER_DYN, "DYN"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_HEADER_ERROR, "ERROR"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_GET_BITS, "GET-BITS"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_NEW_STATE, "NEW-STATE"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_INIT_TREE, "INIT-TREE"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_READ_HEADER, "HDR"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_READ_HEADER_AFTER, "HDR-AFTER"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_BUILD_TREE, "BUILD-TREE"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_DECODE_TREES_STEP1, "DT:STEP1"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_DECODE_TREES_STEP2, "DT:STEP2"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_DECODE_TREES_STEP3, "DT:STEP3"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_DECODE_TREES_STEP3_SYM, "DT:STEP3-SYM"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_DECODE_SYMBOL, "DEC-SYM"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_COPY_DATA, "COPY-DATA"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_BLOCK_DATA, "BLOCK-DATA"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_BLOCK_DATA_STEP2, "BLOCK-DATA:STEP2"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_BLOCK_DATA_STEP3, "BLOCK-DATA:STEP3"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_BUILD_TREE_DT, "BUILD-TREE:DT"),
    INFLATE_AIO_STEP_DESC(INFLATE_AIO_STATE_BUILD_FIXED_TREES, "BUILD-FIXED-TREE")
    /**/
};
const char *unknown_step_name = "Unknown";

const char *get_step_name(int step)
{
    size_t i;
    const size_t limit = sizeof(step_names) / sizeof(step_names[0]);
    for (i = 0; i < limit; i++)
    {
        if (step_names[i].step == step)
        {
            return step_names[i].name;
        }
    }
    return unknown_step_name;
}

/**
 *
 */
int inflate_aio(unsigned char *out_data, unsigned int out_size, unsigned char *in_data, unsigned int in_size)
{
#if INFLATE_AIO_ALLOW_NO_DESTINATION
    if ((in_data == (char *)0) || (in_size == 0))
    {
        return INFLATE_ERROR_PARAMS;
    }
#else
    if ((in_data == (unsigned char *)0) || (in_size == 0) || (out_data == (unsigned char *)0) || (out_size == 0))
    {
        return INFLATE_ERROR_PARAMS;
    }
#endif

    if (sizeof(char *) == 8)
    {
        uint64_t space_left = (uint64_t)in_data;
        space_left = ~space_left; /* simple subtraction from max value :-) */
        if (space_left < (uint64_t)in_size)
        {
            return INFLATE_ERROR_PARAMS;
        }
#if INFLATE_AIO_ALLOW_NO_DESTINATION
        if (out_data == (char *)0)
        {
            out_size = 0;
        }
        else
        {
            space_left = (uint64_t)out_data;
            space_left = ~space_left;
            if (space_left < (uint64_t)out_size)
            {
                return 10;
            }
            if (in_data <= out_data && in_data + in_size)
            {
            }
        }
#else
        space_left = (uint64_t)out_data;
        space_left = ~space_left;
        if (space_left < (uint64_t)out_size)
        {
            return INFLATE_ERROR_PARAMS;
        }
        if (in_data <= out_data && in_data + in_size >= out_data)
        {
            return INFLATE_ERROR_PARAMS;
        }
        else if (out_data + out_size > in_data)
        {
            return INFLATE_ERROR_PARAMS;
        }

#endif
    }
    else
    {
        // 32-bit or any other not supported
        return INFLATE_ERROR_NOT_IMPLEMENTED;
    }

    INFLATE_ALGO_STEP_PTR(ALGO_STEP_START_IN_PTR, in_data, "");
    INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_START_IN_SIZE, in_size, "");
    INFLATE_ALGO_STEP_PTR(ALGO_STEP_START_OUT_PTR, out_data, "");
    INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_START_OUT_SIZE, out_size, "");

    int last = 0, error = 0;

    const unsigned char *state_source = (const unsigned char *)in_data;
    const unsigned char *state_source_end = state_source + in_size;

    unsigned char *state_destination = (unsigned char *)out_data;
    const unsigned char *state_destination_start = state_destination;
    const unsigned char *state_destination_end = state_destination + out_size;

    int state_bitcount = 0;
    unsigned int state_bitbuf = 0;

    int out_available = out_size, in_available = in_size;

    /* These 2 are reserved inputs for get bits! */
    unsigned int in_bits = 0, offs = 0;

    unsigned char *copy_src, *copy_dest;
    int copy_length = 0;

    struct inflate_tree *lt, *dt, *cur_tree;

    unsigned int hlit = 0, hdist = 0, hclen = 0;

    unsigned char *p_build_tree_len = lengths;
    int build_tree_size = 0;

    int last_sym = 0;
    int block_data_length = 0;
    int decode_trees_idx = 0;

    unsigned int comp_state = INFLATE_AIO_STATE_READ_HEADER;

    do
    {

        if (state_source_end > state_source)
        {
            in_available = state_source_end - state_source;
        }
        else
        {
            /* Can't about here,
             * there might be some bits in 'bit buffer' for END-OF-BLOCK.
             */
            in_available = 0;
        }

        if (state_destination_end > state_destination)
        {
            out_available = state_destination_end - state_destination;
        }
        else
        {
            /* Can't abort here, because there might be end-of-block input */
            out_available = 0;
        }

        /* Extract state to process. */
        unsigned int cur_state = comp_state & INFLATE_AIO_STATE_BITMASK;

        comp_state >>= INFLATE_AIO_STATE_BITSHIFT;

#if USE_ALGO_STEP_LOGGING
        if (cur_state < 4)
        {
            INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_CHANGE_TO, cur_state, "");
        }
#endif

        switch (cur_state)
        {
        case INFLATE_AIO_STATE_HEADER_RAW:
        {
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_RAW);
            INFLATE_ALGO_STEP(ALGO_STEP_RAW);
            /* read and process */

            unsigned int ui32_val = *((unsigned int *)state_source);
            state_source += 4;
            in_available -= 4;

#if INFLATE_DO_CHECK_FOR_ERRORS
            register unsigned int temp = ((unsigned int)(ui32_val >> 16));
            temp = (~temp) & 0xFFFF;
            ui32_val &= 0xFFFF;

            if (temp != ui32_val)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "RAW:size-check");
                return INFLATE_ERROR_DATA;
            }
#else
            ui32_val &= 0xFFFF;
#endif

#if DO_OVERFLOW_TEST
            // Overflows?
            if ((int)ui32_val > out_available || (int)ui32_val > in_available)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "RAW:overflow");
                return INFLATE_ERROR_DATA;
            }
#endif
            memcpy(state_destination, state_source, (size_t)ui32_val);
            out_available -= ui32_val;
            in_available -= ui32_val;

            state_source += ui32_val;
            state_destination += ui32_val;

            state_bitbuf = 0;
            state_bitcount = 0;

            comp_state = INFLATE_AIO_STATE_NEW_STATE;
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_HEADER_FIXED:
        {
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_FIXED);
            INFLATE_ALGO_STEP(ALGO_STEP_FIXED_TREE);
            lt = &fixed.both.length_tree;
            dt = &fixed.both.distance_tree;
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
            memset(&dynamic, 0, sizeof(dynamic));
            lt = &dynamic.both.length_tree;
            lt->max_sym = -1;
            dt = &dynamic.both.distance_tree;
            dt->max_sym = -1;

            /* read 14 bits (for hlit, hdist, hclen) */
            in_bits = 14;
            offs = 0;
            INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_GET_BITS, 14, "DECODE-TREES");
            comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_STATE_GET_BITS, INFLATE_AIO_STATE_DECODE_TREES_STEP1, INFLATE_AIO_STATE_DECODE_TREES_STEP2);
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_HEADER_ERROR:
            INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_ERROR);
            return INFLATE_ERROR_DATA;
        case INFLATE_AIO_STATE_BUILD_TREE:
        {
            // printf("BUILD-TREE: %s\n", (cur_tree == lt) ? "LENGTH" : (cur_tree == dt ? "DISTANCE" : "UNKNOWN"));

            memset(cur_tree, 0, sizeof(*cur_tree));
            cur_tree->max_sym = -1;

            int max_sym = cur_tree->max_sym;
            unsigned short *pC = cur_tree->code_counts.us16;
            unsigned char *pLengths = p_build_tree_len;

            register int i;
            register const int limit = build_tree_size;
            /* Count number of codes for each non-zero length */
            for (i = 0; i < limit; ++i)
            {
                unsigned int l = pLengths[i];

                if (l != 0)
                {
                    INFLATE_ALGO_STEP_UI32_UI32_VALUE(ALGO_STEP_BT_CODES, i, l, "");

                    if (l > 15)
                    {
                        INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "(l>15)");
                        return INFLATE_ERROR_DATA;
                    }
                    max_sym = i;
                    pC[l]++;
                }
            }

#if INFLATE_DO_CHECK_FOR_ERRORS
            if (max_sym == -1)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "ERROR: max_sym=-1");
                return INFLATE_ERROR_DATA;
            }
#endif
            cur_tree->max_sym = max_sym;

            /* No need to clear offsets, all values will be assigned */
            unsigned int num_codes = 0, available = 1;

            /* Perform validity check first */

            for (i = 0; i < 16; ++i)
            {
                unsigned int used = *pC;
                ++pC;

                INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_BT_USED_AVAIL, used, available, "");

#if INFLATE_DO_CHECK_FOR_ERRORS
                /* Number of available should <= number of used */
                if (used > available)
                {
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "BT: used > avail");
                    return INFLATE_ERROR_DATA;
                }
#endif
                available = (available - used) << 1;

                INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_BT_OFFSETS, i, num_codes, "");
                offsets.us16[i] = num_codes;
                num_codes += used;
            }
            INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_BT_OFFSETS, i, num_codes, "end");

            /*
             * All codes used?
             * And the special case of only one code that it has length 1.
             */
            if ((num_codes > 1 && available > 0) || (num_codes == 1 && cur_tree->code_counts.us16[1] != 1))
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "BT: num_codes...");
                return INFLATE_ERROR_DATA;
            }

            pC = cur_tree->symbols.us16;
            /* Fill in symbols sorted by code */
            for (i = 0; i < build_tree_size; ++i)
            {
                register unsigned int l = pLengths[i];

                if (l != 0)
                {
                    INFLATE_ALGO_STEP_UI32_UI32_VALUE(ALGO_STEP_BT_CODES2, i, l, "");
                    unsigned short j = offsets.us16[l];
                    pC[j] = i;
                    INFLATE_ALGO_STEP_UI32_UI32_VALUE(ALGO_STEP_BT_CODES3, j, i, "");
                    offsets.us16[l]++;
                    INFLATE_ALGO_STEP_UI32_UI32_VALUE(ALGO_STEP_BT_CODES4, l, offsets.us16[l], "");
                }
            }

            INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_BT_CODES5, num_codes, cur_tree->max_sym, "");
            /*
             * For the special case of only one code (which will be 0) add a
             * code 1 which results in a symbol that is too large
             */
            if (num_codes == 1)
            {
                cur_tree->code_counts.us16[1] = 2;
                // t->symbols.us16[1] = t->max_sym + 1;
                pC[1] = cur_tree->max_sym + 1;
            }
            // printf("BUILD-TREE-END: %s\n", (cur_tree == lt) ? "LENGTH" : (cur_tree == dt ? "DISTANCE" : "UNKNOWN"));
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_DECODE_TREES_STEP1:
        {
            /* 5 bits for HLIT (257-286) # of Literal/Length codes  */
            hlit = ((unsigned int)(in_bits & 0x1F)) + 257;
            in_bits >>= 5;
            /* 5 bits for HDIST (1-32) # of Distance codes */
            hdist = ((unsigned int)(in_bits & 0x1F)) + 1;
            in_bits >>= 5;
            /* 4 bits for HCLEN (4-19) # of Code Length codes */
            hclen = in_bits + 4;

            INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, hlit, "hlit");
            INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, hdist, "hdist");
            INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, hclen, "hclen");
            /*
             * The RFC limits the range of HLIT to 286.
             * HDIST has range 1-32.
             * HDIST 31 and 32 (or without +1 : 30,31) have no meaning.
             * We treat them as errors here.
             *
             * zlib does it also: https://github.com/madler/zlib/issues/82
             */
            if (hlit > 286 || hdist > 30)
            {
                INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "(hlit > 286 || hdist > 30)");
                return INFLATE_ERROR_DATA;
            }

            INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_DECODE_TREES_READ_CODE_LENGTHS, hclen, "");
            in_bits = (hclen * 3) | 0x40; /* at most 57 */
            in_bits |= (12 << 8);
            offs = 0;
            comp_state = INFLATE_AIO_STATE_COMBINE2(INFLATE_AIO_STATE_GET_BITS, INFLATE_AIO_STATE_DECODE_TREES_STEP2);
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_DECODE_TREES_STEP2:
        {
            // inflate_nullify(&dynamic, sizeof(dynamic));
            inflate_nullify(&lengths, sizeof(lengths));
            /*
             * (HCLEN + 4) x 3 bits: code lengths for the code length
             * alphabet: 16, 17, 18, 0,
             * 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
             */
            /* We don't use any lookup array/table and compute the lengths */
            register unsigned int temp = in_bits;

            int i;
            /* bit buffer should already contain at least 12 bits */
            for (i = 0; i < 3; ++i)
            {
                lengths[i + 16] = (unsigned char)(temp & 0x7);
                INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_DECODE_TREES_GET_BITS, i, (temp & 0x7), "");
                INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_DECODE_TREES_GET_BITS, i + 16, (temp & 0x7), "");
                temp >>= 3;
            }
            lengths[0] = (unsigned char)(temp & 0x7);
            INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_DECODE_TREES_GET_BITS, i, (temp & 0x7), "");
            INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_DECODE_TREES_GET_BITS, 0, (temp & 0x7), "");
            temp >>= 3;
            i++;

            /* We pack indexes into length array into a single 64-bit value */
            register uint64_t offset_container = INFLATE_AIO_LENGTH_OFFSETS_PACKED;

            register const unsigned char *pSrc = state_source;
            register int itemp = state_bitcount;
            const int limit = hclen;
            temp = state_bitbuf;

            for (; i < limit; ++i)
            {
                /* Need 3 bits per length */
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
                INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_DECODE_TREES_GET_BITS, i, (temp & 0x7), "");
                INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_DECODE_TREES_GET_BITS, k, (temp & 0x7), "");
                temp >>= 3;
                itemp -= 3;
            }
            state_source = pSrc;
            state_bitbuf = temp;
            state_bitcount = itemp;

            /* Build code length tree (in literal/length tree to save space) */
            cur_tree = lt;
            cur_tree->max_sym = -1;
            p_build_tree_len = lengths;
            build_tree_size = 19;
            decode_trees_idx = 0;

            comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_STATE_BUILD_TREE, INFLATE_AIO_STATE_DECODE_TREES_STEP3, INFLATE_AIO_STATE_BLOCK_DATA);
        }
        break;
        case INFLATE_AIO_STATE_DECODE_TREES_STEP3:
        {
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
            // int d = hlit + hdist - decode_trees_idx;
            // printf("CHECK: %d < %d => %d\n", decode_trees_idx, hlit + hdist , d > 0 ? 1 : 0);
            if (decode_trees_idx < hlit + hdist)
            {
                cur_tree = lt;
                comp_state = INFLATE_AIO_STATE_COMBINE2(INFLATE_AIO_STATE_DECODE_TREES_STEP3, INFLATE_AIO_STATE_BLOCK_DATA);
                comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_DECODE_SYMBOL, INFLATE_AIO_STATE_DECODE_TREES_STEP3_SYM, comp_state);
            }
            else
            {
                INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_DT_ITER_END, decode_trees_idx, hlit + hdist, "");
                /* Is end-of-block symbol present? */
#if INFLATE_DO_CHECK_FOR_ERRORS
                if (lengths[256] == 0)
                {
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "DT3: lengths[256]==0");
                    return INFLATE_ERROR_DATA;
                }
#endif
                cur_tree = lt;
                p_build_tree_len = lengths;
                build_tree_size = hlit;
                comp_state = INFLATE_AIO_STATE_COMBINE2(
                    INFLATE_AIO_STATE_BLOCK_DATA, INFLATE_AIO_STATE_NEW_STATE);

                comp_state = INFLATE_AIO_STATE_COMBINE4(
                    INFLATE_AIO_STATE_BUILD_TREE,
                    INFLATE_AIO_STATE_BUILD_TREE_DT,
                    INFLATE_AIO_STATE_BUILD_TREE, comp_state);
            }
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_DECODE_TREES_STEP3_SYM:
        {
#if INFLATE_DO_CHECK_FOR_ERRORS
            /* Check code length tree is not empty */
            if (last_sym > lt->max_sym)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "STEP3:DECODE-SYMBOL");
                return INFLATE_ERROR_DATA;
            }
#endif
            /*
             * in "cases" we store offsets in lowest 4 bits
             * and number of additional bits in next bits (at most 3)
             */
            int sym = last_sym;
            int num = decode_trees_idx;

            INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_DT_SYM, num, sym, "");

            int length = 1;

#if 0
            if (sym > 15)
            {
                if (sym == 16)
                {
#if INFLATE_DO_CHECK_FOR_ERRORS
                    if (num != 0)
                    {
                        sym = lengths[num - 1];
                    }
                    else
                    {
                        INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "DT3-SYM: sym=16");
                        return INFLATE_ERROR_DATA;
                    }
#else
                    sym = lengths[num - 1];
#endif
                }
                else if (sym < 19)
                {
                    sym = 0;
                    length &= 0x3;
                    length <<= 3;
                    /* 16: Repeat code length 0 for 3-6 times (read 2 bits) */
                    /* 17: Repeat code length 0 for 3-10 times (read 3 bits) */
                    /* 18: Repeat code length 0 for 11-138 times (read 7 bits) */
#define SAOS (((2 << 4) | (3)) | (((3 << 4) | (3)) << 8) | (((7 << 4) | (11)) << 16))
                    length = SAOS >> length;
                }
            }
#else

            switch (sym)
            {
            case 16:
                /* Copy previous code length 3-6 times (read 2 bits) */
#if INFLATE_DO_CHECK_FOR_ERRORS
                if (num == 0)
                {
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "DT3-SYM: sym=16");
                    return INFLATE_ERROR_DATA;
                }
#endif
                sym = lengths[num - 1];
                length = ((2 << 4) | (3));
                break;
            case 17:
                /* Repeat code length 0 for 3-10 times (read 3 bits) */
                sym = 0;
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
#endif

            if (length != 1)
            {
                register int itemp = state_bitcount;
                register unsigned int temp = state_bitbuf;

                int offset = length & 0xF;
                length >>= 4;

                /* Need more bits than in bit buffer? */
                if (length > itemp)
                {
#if INFLATE_DO_CHECK_FOR_ERRORS
                    if (state_source >= state_source_end)
                    {
                        INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "DT3-SYM: bits error");
                        return INFLATE_ERROR_BUFFER;
                    }
#endif
                    temp |= (unsigned int)*state_source << itemp;
                    ++state_source;
                    itemp += 8;
                }
                in_bits = (0x1 << length) - 1;
                in_bits = (temp & in_bits) + offset;
                itemp -= length;
                temp >>= length;
                length = in_bits;

                state_bitcount = itemp;
                state_bitbuf = temp;
            }
            INFLATE_ALGO_STEP_I32_I32_VALUE(ALGO_STEP_DT_SYM2, sym, length, "");

#if INFLATE_DO_CHECK_FOR_ERRORS
            if ((unsigned)length > hlit + hdist - num)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "DT3-SYM: length >...");
                return INFLATE_ERROR_DATA;
            }
#endif

            while (length-- != 0)
            {
                lengths[num++] = sym;
            }

            INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_DT_ITER, num, "");
            decode_trees_idx = num;
        }
        break;
            /* -- */

        case INFLATE_AIO_STATE_BUILD_TREE_DT:
        {
            printf("BT: DISTANCE!\n");
            cur_tree = dt;
            p_build_tree_len = lengths + hlit;
            build_tree_size = hdist;
            comp_state = INFLATE_AIO_STATE_COMBINE3(
                INFLATE_AIO_STATE_BUILD_TREE,
                INFLATE_AIO_STATE_BLOCK_DATA, INFLATE_AIO_STATE_NEW_STATE);
        }
        break;
        case INFLATE_AIO_STATE_BLOCK_DATA:
            /* From RFC 1951:
             *  loop (until end of block code recognized)
             *    decode literal/length value from input stream
             *    if value < 256
             *       copy value (literal byte) to output stream
             *    otherwise
             *       if value = end of block (256)
             *          break from loop
             *       otherwise (value = 257..285)
             *          decode distance from input stream
             *         move backwards distance bytes in the output
             *         stream, and copy length bytes from this
             *         position to the output stream.
             * end loop
             */
            {
                cur_tree = lt;
                comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_DECODE_SYMBOL, INFLATE_AIO_STATE_BLOCK_DATA_STEP2, INFLATE_AIO_STATE_BLOCK_DATA_STEP3);
            }
            break;
            /* Decode block using decoded trees */
        case INFLATE_AIO_STATE_BLOCK_DATA_STEP2:
        {
            int sym = last_sym; /* from decode symbol */

            if (sym > 285)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "BLOCK-DATA2: sym >285 error");
                return INFLATE_ERROR_DATA;
            }
            /* Check for end of block */
            if (sym == 256)
            {
                comp_state = INFLATE_AIO_STATE_NEW_STATE;
                break;
            }

            if (state_destination >= state_destination_end)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "BLOCK-DATA2: dest error");
                return INFLATE_ERROR_DEST;
            }

            // literal 0..255?
            if (sym < 256)
            {
                INFLATE_ALGO_STEP_UI32_PTR_VALUE(ALGO_STEP_BD_WRITE_RAW, sym, state_destination, "");

                *state_destination = sym;
                ++state_destination;
                comp_state = INFLATE_AIO_STATE_BLOCK_DATA;
            }
            else
            {
#if INFLATE_DO_CHECK_FOR_ERRORS
                /* Check sym is within range and distance tree is not empty */
                if (sym > lt->max_sym || dt->max_sym == -1)
                {
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "BLOCK-DATA2: sym error");
                    return INFLATE_ERROR_DATA;
                }
#endif
                sym -= 257;

#define CL 1

#if CL
                int length = sym + 3;
#else
                int length = length_base[sym];
#endif
                /* Extra bits and base tables for length codes */
                // static const unsigned char length_bits[29] = {
                // 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                // 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                // 4, 4, 4, 4, 5, 5, 5, 5, 0};
                /* We compute bits instead of lookup */
                if (sym <= 8)
                {
                    sym >>= 3;
                }
                else
                {
                    if (sym == 28)
                    {
                        sym = 0;
#if CL
                        length = 258;
#endif
                    }
                    else
                    {
                        sym -= 8;
#if CL
#if 1
                        // Added already: length=sym+3, so just add the missing part...
                        /* 28, 29, 30, 31 */
                        uint64_t v = INFLATE_CHARS_AS_UINT64(135, 166, 197, 227ul, 0ul, 0ul, 0ul, 0ul);

                        if (sym < 16)
                        {
                            if (sym < 8)
                            {
                                v = INFLATE_CHARS_AS_UINT64(1, 2, 3, 4ul, (7ul), (10ul), (13ul), (16ul));
                            }
                            else
                            {
                                /* 20, 21, 22, 23, 24, 25, 26, 27 */
                                v = INFLATE_CHARS_AS_UINT64(23, 30, 37ul, 44, (59ul), (74ul), (89ul), (104ul));
                            }
                        }

                        unsigned int s = (sym & 0x7) << 3;
                        length += (v >> s);
#else
                        uint64_t v = sym >> 3;
                        v = inflate_length_base64[v];
                        unsigned int s = (sym & 0x7) << 3;
                        length += (v >> s);
#endif

#endif
                        sym >>= 2;
                        sym++;
                    }
                }

                if (sym != 0)
                {
                    /* Possibly get more bits from length code */
                    // length = inflate_getbits(d, sym, length);
                    register int itemp = state_bitcount;
                    register unsigned int temp = state_bitbuf;

                    while (itemp < sym)
                    {
                        if (state_source < state_source_end)
                        {
                            temp |= (unsigned int)*state_source << itemp;
                            state_source++;
                        }
                        else
                        {
                            INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "BLOCK-DATA2: bits error");
                            return INFLATE_ERROR_BUFFER;
                        }
                        itemp += 8;
                    }

                    length += (temp & (unsigned int)((0x1 << sym) - 1));
                    temp >>= sym;
                    itemp -= sym;
                    state_bitbuf = temp;
                    state_bitcount = itemp;
                }
                cur_tree = dt;
                block_data_length = length;
                comp_state = INFLATE_AIO_STATE_COMBINE2(INFLATE_AIO_DECODE_SYMBOL, INFLATE_AIO_STATE_BLOCK_DATA_STEP3);
            }
        }
        break;
        case INFLATE_AIO_STATE_BLOCK_DATA_STEP3:
        {
            // int dist = inflate_decode_symbol(d, dt);
            int dist = last_sym;

#if INFLATE_DO_CHECK_FOR_ERRORS
            /* dist is within range? (dist 30,31 are invalid) */
            if (dist > 29 || dist > dt->max_sym)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "(dist > 29 || dist > dt->max_sym)");
                return INFLATE_ERROR_DATA;
            }
#endif
            int data_offset;

            if (dist < 4)
            {
                data_offset = dist + 1;
                dist = 0;
            }
            else
            {
#if 1
                data_offset = dist_base[dist];
                dist -= 4;
                dist >>= 1;
                dist++;
#else
                dist -= 4;
                const unsigned int o = dist & 0x1;
                dist >>= 1;
                dist++;
                data_offset = dist + 1;
                data_offset = 0x1 << data_offset;
                data_offset |= (data_offset >> o);
                data_offset++;
#endif

                /* Possibly get more bits from distance code */
                // data_offset = inflate_getbits(d, dist, data_offset);
                register int itemp = state_bitcount;
                register unsigned int temp = state_bitbuf;
                while (itemp < dist)
                {
                    if (state_source < state_source_end)
                    {
                        temp |= (unsigned int)*state_source << itemp;
                        state_source++;
                        itemp += 8;
                    }
                    else
                    {
                        INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "BD: bits error.");
                        return INFLATE_ERROR_BUFFER;
                    }
                }
                data_offset += temp & ((0x1 << dist) - 1);
                temp >>= dist;
                itemp -= dist;
                state_bitcount = itemp;
                state_bitbuf = temp;
            }

#if INFLATE_DO_CHECK_FOR_ERRORS
            int64_t addr_dist = (state_destination - state_destination_start);

            /* Possible to go back? */
            if (data_offset > (int)addr_dist)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "(data_offset > addr_dist)");
                return INFLATE_ERROR_DATA;
            }

            int length = block_data_length;

            addr_dist = (state_destination_end - state_destination);

            if ((int)addr_dist < length)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "BLOCK-DATA: no dest space.");
                return INFLATE_ERROR_BUFFER;
            }
#endif

            cur_tree = lt;

#if 0
#if 0
      int i;

      unsigned char *pSrc = d->destination - offs;

      /* Copy match */
      for (i = 0; i < length; ++i)
      {
        d->destination[i] = pSrc[i];
      }
#else
            inflate_copy_data(state_destination, state_destination - data_offset, length);

#endif
        comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_DECODE_SYMBOL, INFLATE_AIO_STATE_BLOCK_DATA_STEP2, INFLATE_AIO_STATE_BLOCK_DATA_STEP3);
#else
            //
            copy_src = state_destination - data_offset;
            copy_dest = state_destination;
            copy_length = length;

            comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_DECODE_SYMBOL, INFLATE_AIO_STATE_BLOCK_DATA_STEP2, INFLATE_AIO_STATE_BLOCK_DATA_STEP3);
            comp_state = INFLATE_AIO_STATE_COMBINE3(INFLATE_AIO_COPY_DATA, 28, comp_state);
#endif
        }
        break;
        case 28:
            state_destination += copy_length;
            break;
            /* -- */
        case INFLATE_AIO_STATE_NEW_STATE:
            // read new block?
            // if last was set, then we already computed the last block.
            error = last;
            comp_state = INFLATE_AIO_STATE_READ_HEADER;
            break;
        /* -- */
        case INFLATE_AIO_STATE_GET_BITS:
        {
            register int itemp = state_bitcount;

#if INFLATE_DO_CHECK_FOR_ERRORS
            /* Only checking ? */
            if ((in_bits & 0x40) != 0)
            {
#if INFLATE_DO_CHECK_FOR_ERRORS
                int total = (int)(in_bits & 0x3F);

                if (total > itemp)
                {
                    if (state_source >= state_source_end)
                    {
                        INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "CHECK-BITS:overflow");
                        return INFLATE_ERROR_DATA;
                    }
                    total -= itemp;
                    total = (total + 7) >> 3; /* total bytes ... */

                    if ((state_source_end - state_source) <= total)
                    {
                        INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "total_bits");
                        return INFLATE_ERROR_DATA;
                    }
                }
#endif
                in_bits >>= 8;
            }
#endif
#if INFLATE_DO_CHECK_FOR_ERRORS
            // We read no more than 24 bits at once.
            if (in_bits > 24)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "in_bits>24");
                return INFLATE_ERROR_DATA;
            }
#endif
            itemp = state_bitcount;
            register unsigned int temp = state_bitbuf;

            register const unsigned char *pCur2 = state_source;

#if INFLATE_DO_CHECK_FOR_ERRORS
            register const unsigned char *pEnd2 = state_source_end;
#endif

            // color_text(TEXT_EFFECT_BLINK|COLOR_CYAN, "");
            // printf("GET:%d\n", in_bits);

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
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_BUFFER, "GET-BITS");
                    return INFLATE_ERROR_BUFFER;
                }
#else
                temp |= (unsigned int)*pCur2 << itemp;
                ++pCur2;
#endif
                itemp += 8;
            }
            state_source = pCur2;
            /* minus requested bits */
            itemp -= in_bits;
            state_bitcount = itemp;

            /* Bits from bit buffer! */
            itemp = temp & (unsigned int)((0x1 << in_bits) - 1);

            /* store for future requests */
            temp >>= in_bits;
            state_bitbuf = temp;

            in_available = (state_source_end - state_source);

            in_bits = (itemp + offs);
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_READ_HEADER:
        {
            INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_GET_BITS, 3, "NEW-STATE");
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
            INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, in_bits, "NEW-STATE");
            comp_state = in_bits;
            last = comp_state & 0x1;
            comp_state >>= 1;
            // INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_CHANGE_TO, comp_state, "");
            if (comp_state > 2)
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "Invalid next state!");
                return INFLATE_ERROR_DATA;
            }
        }
        break;
        /* -- */
        case INFLATE_AIO_STATE_BUILD_FIXED_TREES:
        {
            lt = &fixed.both.length_tree;
            dt = &fixed.both.distance_tree;

            if (0 == fixed_done)
            {
                INFLATE_ALGO_STEP(ALGO_STEP_BUILD_FIXED_TREE);

                inflate_nullify(&fixed.both, sizeof(fixed));

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
                uint64_t *pUI64 = lt->symbols.ui64;

#define EX_ENCODE 0

#if EX_ENCODE
                /* 256,0,280,144 -> /8 = 32,0,35,18 = 6 bits  required */
                /* 6,36,2,28 -> 3,18,1,14 : 5 bits required */
                /* Hard to pack, so use 64 bits */
                uint64_t sizes_and_offsets = ((6UL) | (256UL << 6) | (36UL << 16) | (0) | (2UL << 32) | (280UL << 38) | (28UL << 48) | (144UL << 56));
                while (sizes_and_offsets != 0)
                {
                    itemp = (int)(sizes_and_offsets & 0x3F);
                    sizes_and_offsets >>= 6;

                    register uint64_t a = (sizes_and_offsets & 0x3FF);
                    sizes_and_offsets >>= 10;
                    a |= (a << 16);
                    a |= (a << 32);
                    a += ADD0_INC; /* |= also possible, all 'short parts' have 3 lowest bits 0 */

                    for (; itemp != 0; itemp--)
                    {
                        *pUI64 = a;
                        a += ADD1;
                        ++pUI64;
                    }

                    temp >>= 8;
                }
#else
                const uint64_t *pA = inflate_adds;
                register unsigned int temp = (6 | (36 << 8) | (2 << 16) | (28 << 24));

                while (temp != 0)
                {
                    register uint64_t a = *pA;
                    ++pA;
                    register int itemp = (int)(temp & 0xFF);

                    for (; itemp != 0; itemp--)
                    {
                        *pUI64 = a;
                        a += ADD1;
                        ++pUI64;
                    }

                    temp >>= 8;
                }
#endif
#else
                unsigned short *pS = cur_tree->symbols.us16;
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
                register uint64_t a = UI64_REPEAT_INC(0UL);
                register int itemp = 8;

                for (; itemp > 0; itemp--)
                {
                    *pUI64 = a;
                    a += ADD1;
                    ++pUI64;
                }
#else
                for (i = 0; i < 32; ++i)
                {
                    cur_tree->symbols.us16[i] = i;
                }
#endif
                ++fixed_done;
            }
        }
        break;
        /* -- */
        case INFLATE_AIO_DECODE_SYMBOL:
        {
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
            int base = 0, offs = 0, len;

            unsigned short *pC = &cur_tree->code_counts.us16[1];
            register int itemp = state_bitcount;
            register unsigned int temp = state_bitbuf;

            for (len = 1;; ++len)
            {
                if (len > 15)
                {
                    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "DS:len>15");
                    return INFLATE_ERROR_DATA;
                }
                /* get 1 bit */
                if (itemp == 0)
                {
                    if (state_source < state_source_end)
                    {
                        temp = *state_source;
                        state_source++;
                        itemp = 8;
                    }
                    else
                    {
                        INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "DS:no-bits");
                        return INFLATE_ERROR_BUFFER;
                    }
                }
                offs = (offs << 1) | (int)(temp & 0x1);
                temp >>= 1;
                --itemp;

                const int c = *pC;
                ++pC;
                if (offs < c)
                {
                    break;
                }

                base += c;
                offs -= c;
            }
            state_bitbuf = temp;
            state_bitcount = itemp;
            base += offs;

            if ((base < 0) || (base > 287))
            {
                INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, INFLATE_ERROR_DATA, "((base < 0) || (base > 287))");
                return INFLATE_ERROR_DATA;
            }
            else
            {
                last_sym = cur_tree->symbols.us16[base];
            }
        }
        break;
        case INFLATE_AIO_COPY_DATA:
        {
            int i;
            // byte by byte, because of overlaps
            for (i = 0; i < copy_length; ++i)
            {
                copy_dest[i] = copy_src[i];
            }
        }
        break;
        /* -- */
        default:
            // Some invalid internal algorithm state!
            return 1000;
            break;
        }

    } while (error == 0);

    // #include "../colors.h"

    // color_text(TEXT_EFFECT_BLINK|COLOR_MAGENTA, "get symbol mit get bits verschmelzen.\n");

    return 0;
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

#if 0
            pUI64 = offsets.ui64;
            
            for (i = 0; i < 4; ++i)
            {
                uint64_t a = num_codes;
                num_codes += *pC;
                ++pC;
                a |= (((uint64_t)num_codes)<<16);
                num_codes += *pC;
                ++pC;
                a |= (((uint64_t)num_codes)<<32);
                num_codes += *pC;
                ++pC;
                a |= (((uint64_t)num_codes)<<48);
                num_codes += *pC;
                ++pC;
                *pUI64 = a;
                ++pUI64;
            }
#endif