#ifndef INFLATE_INCLUDED
#define INFLATE_INCLUDED

#ifdef INFLATE_OK
#error "INFLATE_OK already defined"
#endif
#ifdef INFLATE_ERROR_PARAMS
#error "INFLATE_ERROR_PARAMS already defined"
#endif

#define INFLATE_OK 0
// error = 1 is reserved
#define INFLATE_ERROR_NUM_TOO_LARGE 2
#define INFLATE_ERROR_PARAMS 2

#define INFLATE_ERROR_DATA 4
#define INFLATE_ERROR_LEN_TOO_LARGE 4  // DATA_ERROR
#define INFLATE_ERROR_USED_TOO_LARGE 4 // DATA_ERROR
#define INFLATE_ERROR_BUFFER 5
#define INFLATE_ERROR_DEST 6

#define DECODE_SYMBOL_ERROR_LEN -100000
#define DECODE_SYMBOL_ERROR_SYM -200000

#define INFLATE_BUILD_FIXED_TREES_ALGO 3
#define DO_CMP 0

#define UI64_REPEAT(C) ((C) | ((C) << 16) | ((C) << 32) | ((C) << 48))
#define UI64_REPEAT_INC(C) ((C) | ((C + 1UL) << 16) | ((C + 2UL) << 32) | ((C + 3UL) << 48))
#define ADD1 UI64_REPEAT(4UL)

#ifndef INFLATE_SKIP_CHECK_FOR_ERRORS
#define INFLATE_SKIP_CHECK_FOR_ERRORS 0
#else
#ifdef INFLATE_DO_CHECK_FOR_ERRORS
#error "INFLATE_DO_CHECK_FOR_ERRORS and INFLATE_SKIP_CHECK_FOR_ERRORS defined. Define only one of them!"
#endif
#endif

#ifndef INFLATE_DO_CHECK_FOR_ERRORS
#if INFLATE_SKIP_CHECK_FOR_ERRORS == 0
#define INFLATE_DO_CHECK_FOR_ERRORS 1
#else
#define INFLATE_DO_CHECK_FOR_ERRORS 0
#endif
#else
#ifdef INFLATE_SKIP_CHECK_FOR_ERRORS
#error "INFLATE_DO_CHECK_FOR_ERRORS and INFLATE_SKIP_CHECK_FOR_ERRORS defined. Define only one of them!"
#endif
#endif

#ifndef SKIP_OVERFLOW_TEST
#define SKIP_OVERFLOW_TEST 0
#define DO_OVERFLOW_TEST 1
#endif

#ifndef GLOBAL_DATA_STRUCTS
#define GLOBAL_DATA_STRUCTS 1
#endif

#if INFLATE_BUILD_FIXED_TREES_ALGO == 3
static const uint64_t inflate_adds[] = {
    UI64_REPEAT_INC(256UL),
    UI64_REPEAT_INC(0UL),
    UI64_REPEAT_INC(280UL),
    UI64_REPEAT_INC(144UL),
};
#endif

struct inflate_tree
{
  int max_sym;
  union
  {
    uint64_t ui64[4];
    unsigned short us16[16];
  } code_counts; /*!< Number of codes with a given length */
  union
  {
    uint64_t ui64[36];
    unsigned short us16[288];
  } symbols; /*!< Symbols sorted by code */
  // unsigned short symbols[288];
};

union inflate_tree_aligned
{
  struct inflate_tree tree;
  unsigned long ul_array[((sizeof(struct inflate_tree) + 7) >> 3)];
};

struct inflate_tree_pair
{
  struct inflate_tree length_tree;
  struct inflate_tree distance_tree;
};

/*!< stores computation state information */
struct inflate_state
{
  int bitcount;        /*!< negative in case of error */
  unsigned int bitbuf; /*!< bit buffer: stores some bits from input */

  // const unsigned char *source_start;
  const unsigned char *source;     /*!< current pointer to source data */
  const unsigned char *source_end; /*!< limit pointer to source data */

  unsigned char *destination_start; /*!< start pointer to result data */
  unsigned char *destination;       /*!< current pointer to result data */
  unsigned char *destination_end;   /*!< limit pointer to result data */
};

#define IS_INVALID_DEST(D) (D->destination >= D->destination_end)

// unsigned int inflate_read_stream_bits(unsigned char *stream, unsigned int *bits_done, unsigned long *pValBuf, int *inc, unsigned int count);
#include <assert.h>

#include "../test_string_buffer.h"

TestStringBuffer b1;

#undef TEST_PRINT

#define TEST_PRINT(FORMAT, ...) ADD_TO_SB(b1, FORMAT, __VA_ARGS__)

#define TEST_PRINTF(FORMAT, ...) printf(FORMAT, __VA_ARGS__)

void print_state1(void *state)
{
  if (state != 0)
  {
    struct inflate_state *s = (struct inflate_state *)state;
    TEST_PRINT("Tag: %x|BC:%d|Overflow: %x\n", s->bitbuf, s->bitcount, s->bitcount < 0 ? 1 : 0);
    TEST_PRINT("S: %p|D: %ld|>%ld\n", s->source, s->destination - s->destination_start, s->destination_end - s->destination);
  }
}

#define print_state print_state1

#define USE_ALGO_STEP_LOGGING 1

#include "inflate_dev.h"

void inflate_nullify(void *p, size_t size)
{
  INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_NULLIFY, size, "");
  memset(p, 0, size);
}

void inflate_clearmem(void *p, size_t len)
{
  inflate_nullify(p, len);
}

static void inflate_init_tree(struct inflate_tree *t)
{
  memset(t, 0, sizeof(struct inflate_tree));
  // inflate_clearmem(t, sizeof(struct inflate_tree));
  t->max_sym = -1;
}

static int inflate_copy_data(unsigned char *dst, unsigned char *src, int length)
{
  int i;
  // byte by byte, because of overlaps
  for (i = 0; i < length; ++i)
  {
    dst[i] = src[i];
  }
  return 0;
}

// See RFC1951 (section 3.2.5)
//       Extra
// Code Bits Length(s)
// ---- ---- ------
//  257   0     3
//  258   0     4
//  259   0     5
//  260   0     6
//  261   0     7
//  262   0     8
//  263   0     9
//  264   0    10
//
//  265   1   11,12
//  266   1   13,14
//  267   1   15,16
//  268   1   17,18
//  269   2   19-22
//  270   2   23-26
//  271   2   27-30
//  272   2   31-34
//
//  273   3   35-42
//  274   3   43-50
//  275   3   51-58
//  276   3   59-66
//  277   4   67-82
//  278   4   83-98
//  279   4   99-114
//  280   4  115-130
//
//  281   5  131-162
//  282   5  163-194
//  283   5  195-226
//  284   5  227-257
//  285   0    258
//

//     Extra           Extra               Extra
// Code Bits Dist  Code Bits   Dist     Code Bits Distance
// ---- ---- ----  ---- ----  ------    ---- ---- --------
//   0   0    1     10   4     33-48    20    9   1025-1536
//   1   0    2     11   4     49-64    21    9   1537-2048
//   2   0    3     12   5     65-96    22   10   2049-3072
//   3   0    4     13   5     97-128   23   10   3073-4096
//   4   1   5,6    14   6    129-192   24   11   4097-6144
//   5   1   7,8    15   6    193-256   25   11   6145-8192
//   6   2   9-12   16   7    257-384   26   12  8193-12288
//   7   2  13-16   17   7    385-512   27   12 12289-16384
//   8   3  17-24   18   8    513-768   28   13 16385-24576
//   9   3  25-32   19   8   769-1024   29   13 24577-32768
// Bits: shift code to right (i.e. div by 2) and subtract 1 (except 0,1)

// Codes: 30,31 not specified, i.e. error.
static const unsigned short inflate_dist_codes[30] = {
    1, 2, 3, 4,
    /**/
    5, 7, 9, 13,
    /**/
    17, 25, 33, 49,
    /**/
    65, 97, 129, 193,
    /**/
    257, 385, 513, 769,
    //
    1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

/* 4,5,6,7,8,9,10,11, 12,13,14,15 */
/* 1,2,3,6,9,16,23,38, 53,84,115,178 */
static const unsigned short inflate_dist_codes2[30];

static struct inflate_tree_pair fixed;
static int fixed_done = 0;

#if GLOBAL_DATA_STRUCTS
static struct inflate_tree_pair dynamic;

static struct inflate_state state;
#endif

unsigned short *inflate_set_inc(unsigned short *p, int len, int offset)
{
  int i;
  for (i = 0; i < len; ++i)
  {
    *p = (unsigned short)(offset + i);
    ++p;
  }
  return p;
}

#if DO_CMP
unsigned short ts[288];

unsigned short ts2[288];
union a
{
  uint64_t ui64[36];
  unsigned short us16[288];
} ts3;
#endif

static void inflate_build_fixed_trees()
{
  INFLATE_ALGO_STEP(ALGO_STEP_FIXED_TREE);

  if (fixed_done != 0)
  {
    return;
  }
  INFLATE_ALGO_STEP(ALGO_STEP_BUILD_FIXED_TREE);

  inflate_nullify(&fixed, sizeof(fixed));

  struct inflate_tree *t = &fixed.length_tree;
  t->max_sym = 285;

#if 0
  unsigned short *pC = &lt->code_counts[7];
  unsigned int v2 = (24|(152<<8)|(112<<16));

  for(i=0; i < 3; i++) {
    *pC = (v2&0xFF);
    v2>>=8;
    ++pC;
  }
#else

  t->code_counts.us16[7] = 24;
  t->code_counts.us16[8] = 152;
  t->code_counts.us16[9] = 112;
#endif

// encoded offsets and lengths
//  24:off:256;(144:off:0);(8:off:280);112:off:144
// #define LAO ((256) | ((144 << 9) | (0x0 << 17)) | ((0x8ul << 26) | (280ul << 34)) | ((112ul << 43) | (144ul << 51)))
#define LAO2 (144 | ((0x8ul << 17) | (280ul << 25)) | ((112ul << 34) | (144ul << 42)))

#if DO_CMP

  memset(ts, 0, sizeof(ts));
  memset(ts2, 0, sizeof(ts2));

  memset(ts3.us16, 0, sizeof(ts3));
  {
    int i;
    uint64_t v = LAO2;
    unsigned short *pS = ts;
    int l = 24, offset = 256;
    do
    {
      for (i = 0; i < l; ++i, ++pS, ++offset)
      {
        // printf("IS:%d|%d\n", i, offset);
        *pS = (unsigned short)(offset);
      }

      l = (int)(v & 0x1FFFF);
      offset = (l >> 8) & 0xFF;
      l &= 0xFF;
      v >>= 17;
    } while (l != 0);
  }
  {
    unsigned short *pS = ts2;
    pS = inflate_set_inc(pS, 24, 256);
    pS = inflate_set_inc(pS, 144, 0);
    pS = inflate_set_inc(pS, 8, 280);
    inflate_set_inc(pS, 112, 144);
  }
  {
    uint64_t *pUI64 = ts3.ui64;
    const uint64_t *pA = inflate_adds;

#if 0
    int sizes[4] = {
        24 / 4,
        144 / 4,
        8 / 4,
        112 / 4};

    for (i = 0; i < 4; i++)
    {
      uint64_t a = *pA;
      int s = sizes[i];

      for (; s != 0; s--)
      {
        // printf("SA:%lx \n", a);
        *pUI64 = a;
        a += ADD1;
        // printf("INC: %d\n", j);
        ++pUI64;
      }
      ++pA;
    }
#else
    unsigned int sizes = (6 | (36 << 8) | (2 << 16) | (28 << 24));

    while (sizes != 0)
    {
      uint64_t a = *pA;
      int s = sizes & 0xFF;
      sizes >>= 8;

      for (; s != 0; s--)
      {
        // printf("SA:%lx \n", a);
        *pUI64 = a;
        a += ADD1;
        // printf("INC: %d\n", j);
        ++pUI64;
      }
      ++pA;
    }
#endif
  }
#endif

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
  uint64_t *pUI64 = t->symbols.ui64;
  const uint64_t *pA = inflate_adds;

  unsigned int sizes = (6 | (36 << 8) | (2 << 16) | (28 << 24));
  uint64_t a;

  while (sizes != 0)
  {
    a = *pA;
    int s = (int)(sizes & 0xFF);

    for (; s != 0; s--)
    {
      *pUI64 = a;
      a += ADD1;
      ++pUI64;
    }
    ++pA;
    sizes >>= 8;
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

#if DO_CMP
  int cr = memcmp(ts, t->symbols.us16, 288);
  printf("CR:1: %d\n", cr);
  cr = memcmp(ts2, t->symbols.us16, 288);
  printf("CR:2: %d\n", cr);
  cr = memcmp(ts3.us16, t->symbols.us16, 288);
  printf("CR:3: %d\n", cr);
  if (cr != 0)
  {
    int k = 0;
    for (; k < 288; k++)
    {
      if (ts3.us16[k] != t->symbols.us16[k])
      {
        printf("@%d: %d <> %d\n", k, ts3.us16[k], t->symbols.us16[k]);
        // break;
      }
    }
  }
#endif

  t = &fixed.distance_tree;
  t->max_sym = 29;
  /* Build fixed distance tree */
  t->code_counts.us16[5] = 32;

#if INFLATE_BUILD_FIXED_TREES_ALGO == 2
  inflate_set_inc(t->symbols.us16, 32, 0);

#elif INFLATE_BUILD_FIXED_TREES_ALGO == 3
  pUI64 = t->symbols.ui64;
  a = UI64_REPEAT_INC(0UL);
  int i = 0;

  for (; i < 8; i++)
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

/**
 * Build a tree from given array of code lengths
 */
static int inflate_build_tree(struct inflate_tree *t, const unsigned char *lengths,
                              unsigned int num)
{

  printf("BT: %p | l=%p | num=%d\n", (void *)t, lengths, num);
  printf("SYM: %d\n", t->max_sym);

#if INFLATE_SKIP_CHECK_FOR_ERRORS == 0
  // t == 0 || (lengths == 0) ||
  if (num > 288)
  {
    return INFLATE_ERROR_PARAMS;
  }
#endif
  inflate_init_tree(t);
  printf("IT!\n");

  unsigned int ms = t->max_sym;
  unsigned short *pC = t->code_counts.us16;
  unsigned int i;
  /* Count number of codes for each non-zero length */
  for (i = 0; i < num; ++i)
  {
    unsigned int l = lengths[i];
    printf("L:%d => %d\n", i, l);

    if (l != 0)
    {
      if (l > 15)
      {
        printf("LTL: @idx=%d | val=%d\n", i, lengths[i]);
        return INFLATE_ERROR_DATA;
      }
      ms = i;
      pC[l]++;
    }
  }
  t->max_sym = ms;
  unsigned short offsets[16];
  unsigned int num_codes = 0, available = 1;

  /* Compute offset table for distribution sort */
  for (i = 0; i < 16; ++i)
  {
    unsigned int used = t->code_counts.us16[i];

#if INFLATE_SKIP_CHECK_FOR_ERRORS == 0
    /* Check length contains no more codes than available */
    if (used > available)
    {
      printf("USED>AVAIL: %d > %d\n", used, available);
      return INFLATE_ERROR_USED_TOO_LARGE; // DATA_ERROR
    }
#endif
    available = (available - used) << 1;

    // printf("U|A:%d: %d|%d\n",i, used, available);

    offsets[i] = num_codes;
    num_codes += used;
  }

  /*
   * Check all codes were used, or for the special case of only one
   * code that it has length 1
   */
  if ((num_codes > 1 && available > 0) || (num_codes == 1 && t->code_counts.us16[1] != 1))
  {
    printf("NC:%d | AVAIL:%d |@1:%d\n", num_codes, available, t->code_counts.us16[1]);
    return INFLATE_ERROR_DATA;
  }

  /* Fill in symbols sorted by code */
  for (i = 0; i < num; ++i)
  {
    register unsigned int l = lengths[i];

    if (l != 0)
    {
      unsigned short j = offsets[l];
      t->symbols.us16[j] = i;
      offsets[l]++;
    }
  }

  /*
   * For the special case of only one code (which will be 0) add a
   * code 1 which results in a symbol that is too large
   */
  if (num_codes == 1)
  {
    t->code_counts.us16[1] = 2;
    t->symbols.us16[1] = t->max_sym + 1;
  }

  return INFLATE_OK;
}

/* Extra bits and base tables for length codes */
static const unsigned char length_bits[30] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
    4, 4, 4, 4, 5, 5, 5, 5, 0, 127};

static const unsigned short length_base[30] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
    15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
    67, 83, 99, 115, 131, 163, 195, 227, 258, 0};

/* Extra bits and base tables for distance codes */
static const unsigned char dist_bits[30] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
    4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
    9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

static const unsigned short dist_base[30] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
    33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
    1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

/* Get num bits from source stream */
unsigned int inflate_getbits(struct inflate_state *d, const int num, unsigned int base)
{
  // assert(num >= 0 && num <= 32);
#if INFLATE_DO_CHECK_FOR_ERRORS
  if (num > 32)
  {
    d->bitcount = -1;
    return 0;
  }
#endif

  int bc = d->bitcount;
  unsigned int bitbuf = d->bitbuf;

  if (bc < num)
  {
#if INFLATE_DO_CHECK_FOR_ERRORS
    if (bc < 0)
    {
      return 0;
    }
#endif

    const unsigned char *end = d->source_end;
    const unsigned char *cur = d->source;

    while (bc < num)
    {
#if INFLATE_DO_CHECK_FOR_ERRORS
      if (cur < end)
      {
        bitbuf |= (unsigned int)*cur << bc;
        ++cur;
      }
      else
      {
        d->bitcount = -1;
        return 0;
      }
#else
      bitbuf |= (unsigned int)*cur << bc;
      ++cur;
#endif
      bc += 8;
    }
    d->source = cur;
  }

#if INFLATE_DO_CHECK_FOR_ERRORS
  // assert(bc <= 32 && num >= 0 && num <= bc);
  if (bc > 32 || num > bc)
  {
    d->bitcount = -1;
    return 0;
  }
#endif
  //
  /* Get bits from bitbuf */
  unsigned int bits = bitbuf & ((0x1UL << num) - 1);

  bitbuf >>= num;
  bc -= num;

  d->bitbuf = bitbuf;
  d->bitcount = bc;

  return (bits + base);
}

/* Given a data stream and a tree, decode a symbol */
static int inflate_decode_symbol(struct inflate_state *d, const struct inflate_tree *t)
{
  int base = 0, offs = 0, len;

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
  for (len = 1; len < 16; ++len)
  {
    // offs = (offs << 1) + inf_getbits(d, 1);
    offs = (offs << 1) | inflate_getbits(d, 1, 0);
#if INFLATE_DO_CHECK_FOR_ERRORS
    if (d->bitcount < 0)
    {
      return 300;
    }
#endif

    const int c = t->code_counts.us16[len];

    if (offs < c)
    {
      break;
    }

    base += c;
    offs -= c;
  }
#if INFLATE_DO_CHECK_FOR_ERRORS
  if (len > 15)
  {
    return 300;
  }
#endif

  base += offs;
  // assert(base + offs >= 0 && base + offs < 288);
#if INFLATE_DO_CHECK_FOR_ERRORS
  if (base < 0 || base > 287)
  {
    return 300;
  }
#endif

  return t->symbols.us16[base];
}

#if 0
/**
 * 1 invalid, 0 valid
 */
int inflate_is_valid_dest(struct inflate_data *d)
{
  if (d->destination >= d->destination_end)
  {
    return 1;
  }
  return 0;
}
#endif

/* Special ordering of code length codes */
static const unsigned char my_clcidx[19] = {
    16, 17, 18, 0, 8,
    7, 9, 6, 10, 5,
    11, 4, 12, 3, 13,
    2, 14, 1, 15};

/* Given a data stream, decode dynamic trees from it */
static int inflate_decode_trees(struct inflate_state *d, struct inflate_tree *lt,
                                struct inflate_tree *dt)
{

  INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_GET_BITS, 14, "DECODE-TREES");
  /* Get 5 bits HLIT (257-286) */
  unsigned int hlit = inflate_getbits(d, 5, 257);

  /* Get 5 bits HDIST (1-32) */
  unsigned int hdist = inflate_getbits(d, 5, 1);

  /* Get 4 bits HCLEN (4-19) */
  unsigned int hclen = inflate_getbits(d, 4, 4);

  INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, hlit, "hlit");
  INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, hdist, "hdist");
  INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_GET_BITS_RESULT, hclen, "hclen");

  // TEST_PRINT("H|%d|%d|%d|\n", hlit, hdist, hclen);

  /*
   * The RFC limits the range of HLIT to 286, but lists HDIST as range
   * 1-32, even though distance codes 30 and 31 have no meaning. While
   * we could allow the full range of HLIT and HDIST to make it possible
   * to decode the fixed trees with this function, we consider it an
   * error here.
   *
   * See also: https://github.com/madler/zlib/issues/82
   */
#if INFLATE_DO_CHECK_FOR_ERRORS
  if (hlit > 286 || hdist > 30)
  {
    // TEST_PRINT("ERROR: hlit=%d | hdist=%d\n", hlit, hdist);
    return INFLATE_ERROR_DATA;
  }
#endif

  unsigned char lengths[288 + 32];
  memset(lengths, 0, 20); // sizeof(lengths));
  unsigned int i, num;

#if 0
  for (i = 0; i < 19; ++i)
  {
    lengths[i] = 0;
  }
#endif

  /* Read code lengths for code length alphabet */
  for (i = 0; i < hclen; ++i)
  {
    /* Get 3 bits code length (0-7) */
    unsigned int clen = inflate_getbits(d, 3, 0);

#if INFLATE_DO_CHECK_FOR_ERRORS
    if (d->bitcount < 0)
    {
      return INFLATE_ERROR_BUFFER;
    }
#endif
    // TEST_PRINT("CLEN:%d\n", (int)clen);
    // print_state(d);

    lengths[my_clcidx[i]] = (unsigned char)clen;
  }

  /* Build code length tree (in literal/length tree to save space) */
#if INFLATE_DO_CHECK_FOR_ERRORS
  int res = inflate_build_tree(lt, lengths, 19);

  if (res != INFLATE_OK)
  {
    return res;
  }
#else
  inflate_build_tree(lt, lengths, 19);
#endif

  /* Check code length tree is not empty */
#if INFLATE_DO_CHECK_FOR_ERRORS
  if (lt->max_sym == -1)
  {
    // printf("ERROR: max sym\n");
    return INFLATE_ERROR_DATA;
  }
#endif

  /* Decode code lengths for the dynamic trees */
  for (num = 0; num < hlit + hdist;)
  {
    int sym = inflate_decode_symbol(d, lt);

    // TEST_PRINT("SYM:%d | %d\n", num, sym);
#if INFLATE_DO_CHECK_FOR_ERRORS
    if (sym > lt->max_sym)
    {
      // printf("ERROR: sym > max sym | %d > %d\n", sym, lt->max_sym);
      return INFLATE_ERROR_DATA;
    }
#endif

    unsigned int length = 1;

    switch (sym)
    {
    case 16:
      /* Copy previous code length 3-6 times (read 2 bits) */
#if INFLATE_DO_CHECK_FOR_ERRORS
      if (num == 0)
      {
        return INFLATE_ERROR_DATA;
      }
#endif
      sym = lengths[num - 1];
      length = inflate_getbits(d, 2, 3);
      break;
    case 17:
      /* Repeat code length 0 for 3-10 times (read 3 bits) */
      sym = 0;
      length = inflate_getbits(d, 3, 3);
      break;
    case 18:
      /* Repeat code length 0 for 11-138 times (read 7 bits) */
      sym = 0;
      length = inflate_getbits(d, 7, 11);
      break;
    default:
      /* Values 0-15 represent the actual code lengths */
      // length = 1;
      break;
    }

#if INFLATE_DO_CHECK_FOR_ERRORS
    if (d->bitcount < 0)
    {
      return INFLATE_ERROR_BUFFER;
    }

    if (length > hlit + hdist - num)
    {
      // printf("ERROR: length | %d > %d + %d - %d (=%d)\n", length, hlit, hdist, num, hlit + hdist - num);

      return INFLATE_ERROR_DATA;
    }
#endif

    while (length-- != 0)
    {
      lengths[num++] = sym;
    }
  }

  /* Is end-of-block symbol present? */
#if INFLATE_DO_CHECK_FOR_ERRORS
  if (lengths[256] == 0)
  {
    return INFLATE_ERROR_DATA;
  }
#endif

  /* Build dynamic trees */
  res = inflate_build_tree(lt, lengths, hlit);

#if INFLATE_DO_CHECK_FOR_ERRORS
  if (res != INFLATE_OK)
  {
    // printf("ERROR: from build tree:hlit %d\n", res);
    return res;
  }
#endif

  return inflate_build_tree(dt, lengths + hlit, hdist);
}

#define INFLATE_CHARS_AS_UINT64(C1, C2, C3, C4, C5, C6, C7, C8) \
  ((C1) | ((C2) << 8) | ((C3) << 16) | ((C4) << 24) | ((C5) << 32) | ((C6) << 40) | ((C7) << 48) | ((C8) << 56))
// static const unsigned short length_base[30] = {
// 3, 4, 5, 6, 7, 8, 9, 10, 11,
// 13, 15, 17, 19, 23, 27, 31, 35,
// 43, 51, 59, 67, 83, 99, 115, 131,
// 163, 195, 227, 258, 0};
static const uint64_t inflate_length_base64[3] = {
    /* 12, 13, 14, 15, 16, 17, 18, 19 */
    INFLATE_CHARS_AS_UINT64(1, 2, 3, 4ul, (7ul), (10ul), (13ul), (16ul)),
    /* 20, 21, 22, 23, 24, 25, 26, 27 */
    INFLATE_CHARS_AS_UINT64(23, 30, 37ul, 44, (59ul), (74ul), (89ul), (104ul)),
    /* 28, 29, 30, 31 */
    INFLATE_CHARS_AS_UINT64(135, 166, 197, 227ul, 0ul, 0ul, 0ul, 0ul),
};

/* Given a stream and two trees, inflate a block of data */
int inflate_block_data(struct inflate_state *d, struct inflate_tree *len_tree,
                       struct inflate_tree *distance_tree)
{
  // From RFC 1951:
  //  loop (until end of block code recognized)
  //    decode literal/length value from input stream
  //    if value < 256
  //       copy value (literal byte) to output stream
  //    otherwise
  //       if value = end of block (256)
  //          break from loop
  //       otherwise (value = 257..285)
  //          decode distance from input stream

  //         move backwards distance bytes in the output
  //         stream, and copy length bytes from this
  //         position to the output stream.
  // end loop
  int result = -1;

  do
  {
    int sym = inflate_decode_symbol(d, len_tree);

    /* Check for overflow in bit reader */
    if (d->bitcount < 0 || sym > 285)
    {
      result = INFLATE_ERROR_DATA;
      break;
    }

    /* Check for end of block */
    if (sym == 256)
    {
      result = INFLATE_OK;
      break;
    }

    if (IS_INVALID_DEST(d))
    {
      result = INFLATE_ERROR_DEST;
      break;
    }

    // literal 0..255?
    if (sym < 256)
    {
      *d->destination = sym;
      d->destination++;
    }
    else
    {
#if INFLATE_DO_CHECK_FOR_ERRORS
      /* Check sym is within range and distance tree is not empty */
      if (sym > len_tree->max_sym || distance_tree->max_sym == -1)
      {
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
        length = inflate_getbits(d, sym, length);
      }

#if INFLATE_DO_CHECK_FOR_ERRORS
      if (d->bitcount < 0)
      {
        result = INFLATE_ERROR_DATA;
        break;
      }
#endif

      int dist = inflate_decode_symbol(d, distance_tree);

#if INFLATE_DO_CHECK_FOR_ERRORS
      /* dist is within range? (dist 30,31 are invalid) */
      if (dist > 29 || dist > distance_tree->max_sym)
      {
        result = INFLATE_ERROR_DATA;
        break;
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
#if 0
        offs = dist_base[dist];
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
        data_offset = inflate_getbits(d, dist, data_offset);
      }

#if INFLATE_DO_CHECK_FOR_ERRORS
      int64_t addr_dist = (d->destination - d->destination_start);

      /* Possible to go back? */
      if (data_offset > (int)addr_dist)
      {
        result = INFLATE_ERROR_DATA;
        break;
      }

      addr_dist = (d->destination_end - d->destination);

      if ((int)addr_dist < length)
      {
        result = INFLATE_ERROR_BUFFER;
        break;
      }
#endif

#if 0
      int i;

      unsigned char *pSrc = d->destination - offs;

      /* Copy match */
      for (i = 0; i < length; ++i)
      {
        d->destination[i] = pSrc[i];
      }
#else
      inflate_copy_data(d->destination, d->destination - data_offset, length);
#endif

      d->destination += length;
    }
  } while (result < 0);

  return result;
}

#define INFLATE_PRINT_CODE_RAW 10
#define INFLATE_PRINT_CODE_FIXED 11
#define INFLATE_PRINT_CODE_DYN 12
#define INFLATE_PRINT_CODE_ERROR 13

#if 1

void my_print_s(int c)
{
  switch (c)
  {
  case INFLATE_PRINT_CODE_RAW:
    printf("RAW!\n");
    return;
  case INFLATE_PRINT_CODE_FIXED:
    printf("FIXED!\n");
    return;
  case INFLATE_PRINT_CODE_DYN:
    printf("DYN!\n");
    return;
  case INFLATE_PRINT_CODE_ERROR:
    printf("ERR(HEADER)!\n");
    return;
  default:
    printf("Unknown code: %d\n", c);
  }
}
#define INFLATE_PRINT_STATE(C) my_print_s(C)

#else
void noop_c(int c) { (void)c; }
#define INFLATE_PRINT_STATE(C) noop(c)
#endif
/**
 *
 */
int inflate(unsigned char *out_data, unsigned int out_size, unsigned char *in_data, unsigned int in_size)
{
  int last = 0, error = 0;

  unsigned int done_bytes = 0, in_done = 0;
  unsigned int cur_length = 0;
  unsigned int temp;

#if GLOBAL_DATA_STRUCTS == 0

  struct inflate_tree_pair dynamic;

  struct inflate_data state;
#endif

  inflate_clearmem(&state, sizeof(state));

  INFLATE_ALGO_STEP_PTR(ALGO_STEP_START_IN_PTR, in_data, "");
  INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_START_IN_SIZE, in_size, "");
  INFLATE_ALGO_STEP_PTR(ALGO_STEP_START_OUT_PTR, out_data, "");
  INFLATE_ALGO_STEP_UI32_VALUE(ALGO_STEP_START_OUT_SIZE, out_size, "");

  state.source = (const unsigned char *)in_data;
  state.source_end = state.source + in_size;

  state.destination = (unsigned char *)out_data;
  state.destination_start = state.destination;
  state.destination_end = state.destination + out_size; //*destLen;

  while (last == 0)
  {
    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_GET_BITS, 3, "NEW-STATE");
    last = inflate_getbits(&state, 1, 0);

    // BTYPE specifies how the data are compressed, as follows:
    // 00 - no compression, 01 - compressed with fixed Huffman codes,
    // 10 - compressed with dynamic Huffman codes, 11 - reserved (error)
    int bits = inflate_getbits(&state, 2, 0);
    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_GET_BITS_RESULT, ((bits << 1) | last), "NEW-STATE");
    INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_CHANGE_TO, bits, "");

#if INFLATE_SKIP_CHECK_FOR_ERRORS == 0
    if (bits == 3)
    {
      error = -1;
      INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "STATE=3");
      break; // error
    }

    if (done_bytes >= out_size || in_done >= in_size)
    {
      error = -100;
      INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "BASE:overflow");
      break;
    }
#endif

    // Uncompressed ?
    if (bits == 0)
    {
      INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_RAW);
      INFLATE_ALGO_STEP(ALGO_STEP_RAW);
      // if(bits_)
      // in_data += (bits_done + 7) >> 3;
      //  reset
      // bits_done = 0;

      // skip any remaining bits in current partially processed byte
      //  skip current bytes
      const unsigned char *pData = state.source;

#if INFLATE_SKIP_CHECK_FOR_ERRORS == 0
      cur_length = *(unsigned int *)pData;
      // printf("CL: %x\n", cur_length);
      temp = ((unsigned int)((cur_length >> 16) & 0xFFFF));
      temp = (~temp) & 0xFFFF;
      cur_length &= 0xFFFF;
#else
      cur_length = *(unsigned short *)in_data;
#endif

#if INFLATE_SKIP_CHECK_FOR_ERRORS == 0
      if (temp != cur_length)
      {
        // printf("EXP:%u | %u , %x\n", cur_length, temp, temp);
        error = -2;
        INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "RAW:size-check");
        break;
      }
#endif

      state.source += 4;

#if SKIP_OVERFLOW_TEST == 0
      // Overflows?
      if (cur_length > out_size - done_bytes || in_done >= in_size || in_done + cur_length > in_size)
      {
        error = -200;
        INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "RAW:overflow");
        break;
      }
#endif
      // temp = cur_length;
      memcpy(state.destination, state.source, (size_t)cur_length);
      state.source += cur_length;
      state.destination += cur_length;
      state.bitbuf = 0;
      state.bitcount = 0;
    }
    else
    {
      // dynamic?
      if (bits == 2)
      {
        INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_DYN);
        INFLATE_ALGO_STEP(ALGO_STEP_DYN_TREE);
        // read representation of code trees
        inflate_init_tree(&dynamic.length_tree);
        inflate_init_tree(&dynamic.distance_tree);

        int res = inflate_decode_trees(&state, &dynamic.length_tree, &dynamic.distance_tree);

        if (res != INFLATE_OK)
        {
          return res;
        }

        /* Decode block using decoded trees */
        error = inflate_block_data(&state, &dynamic.length_tree, &dynamic.distance_tree);

        if (error != 0)
        {
          INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "DYN:block-data");
          return error;
        }
      }
      else
      {
        INFLATE_PRINT_STATE(INFLATE_PRINT_CODE_FIXED);

        inflate_build_fixed_trees();

        error = inflate_block_data(&state, &fixed.length_tree, &fixed.distance_tree);

        if (error != 0)
        {
          INFLATE_ALGO_STEP_I32_VALUE(ALGO_STEP_ERROR, error, "FIXED:block-data");
          return error;
        }
      }
    }
  }

  return error;
}

#include "inflate_aio.h"

#undef print_state

void dev_inflate_exp()
{
  for (int a = 0; a < 26; a++)
  {
    if ((a % 4) == 0)
    {
      printf("\n");
    }
    int b = a;
    unsigned o = b & 1;
    b >>= 1;
    int v = 0x1 << (b + 2);
    v |= (v >> o);
    v++;
    printf("|%d : %d|", a, v);
  }
  printf("\n");
}

#endif