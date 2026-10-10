#ifndef BENCHMARK_INFLATE_ALGORITHMS
#define BENCHMARK_INFLATE_ALGORITHMS

#define DO_CMP 0

#if DO_CMP
unsigned short ts[288];

unsigned short ts2[288];
union a
{
  uint64_t ui64[36];
  unsigned short us16[288];
} ts3;
#endif


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
        a += INFLATE_UI64_ADD1;
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
        a += INFLATE_UI64_ADD1;
        // printf("INC: %d\n", j);
        ++pUI64;
      }
      ++pA;
    }
#endif
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

#endif