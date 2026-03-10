
#define MEMBLOCK_IMPLEMENTATION 1

#include "memblock.h"

#include <inttypes.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <malloc.h>

#include "memblock_info.h"

void DisplayBinary(unsigned int n)
{
  int l = sizeof(n) * 8;
  int j = 0;
  for (int i = l - 1; i >= 0; i--)
  {
    printf("%x", (n & (1 << i)) >> i);
    j++;
    if (j % 4 == 0)
    {
      printf(" ");
    }
  }
}

char binaryGlobalBuf[65];

char *toBinary(uint64_t num, char *buf)
{
  if (buf == (char *)0)
  {
    buf = binaryGlobalBuf;
  }

  buf[64] = '\0';
  buf = buf + 63;
  if (num > 0)
  {
    while (num > 0)
    {
      *buf = (char)(num & 0x1) + '0';
      buf--;
      num >>= 1;
    }
    buf++;
  }
  else
  {
    *buf = '0';
  }
  return buf;
}

int main(void)
{
  size_t a = alignof(max_align_t);
  printf("Alignment of max_align_t is %zu (%#zx)\n", a, a);

  size_t s = 2 * 1024 * 1024;
  // int step = 24;

  float m1 = -1.0f;
  float p1 = 1.0f;
  float zero = 0.0f;

  DisplayBinary(*((unsigned int *)&m1));
  puts("");
  DisplayBinary(*((unsigned int *)&p1));
  puts("");
  DisplayBinary(*((unsigned int *)&zero));
  puts("");

  size_t numElements = s / sizeof(int);
#if 0
  void *p = malloc(s);
  void *p2 = aligned_alloc(32, s);

  size_t realPMem = malloc_usable_size(p);
  size_t realPMem2 = malloc_usable_size(p2);

  

  printf("The address obtained from malloc(%ld) is %#" PRIxPTR " | %#" PRIxPTR "\n", s,
         (uintptr_t)p, (uintptr_t)p2);

  printf("Diffs: 1 = %ld | 2 = %ld\n", (realPMem - s), (realPMem2 - s));

  int *ip = (int *)p;
  int *ip2 = (int *)p2;

  for (int i = 0; i < numElements; i += step)
  {
    *ip = i;
    *ip2 = i - 1;
    ip += step;
    ip2 += step;
  }

  long long diff = (uintptr_t)p - (uintptr_t)p2;
  printf("Diff %lld | %ld\n", diff, numElements);

  printf("Time: %s", ctime(&(time_t){time(NULL)}));
  thrd_sleep(&(struct timespec){.tv_sec = 30}, NULL); // sleep 1 sec
  printf("Time: %s", ctime(&(time_t){time(NULL)}));

  free(p);
  free(p2);
#endif
  printf("\n---MEM---\n");

  size_t sizes[] = {5, 7, 9, 13};
  options_t options[] = {
      MEMBLOCK_OPTION_USE_MUTEX,
      MEMBLOCK_OPTION_ARCH_ALIGNED,
      MEMBLOCK_OPTION_CACHE_ALIGNED,
      MEMBLOCK_OPTION_USE_GLOBAL_MUTEX | MEMBLOCK_OPTION_ARCH_ALIGNED | MEMBLOCK_OPTION_CACHE_ALIGNED};
  numElements = sizeof(sizes) / sizeof(size_t);

  void *pBlocks[sizeof(sizes) / sizeof(size_t)];

  // char buf[65];

  memblockInit();

  for (size_t i = 0; i < numElements; i++)
  {
    // char *pBin = toBinary(options[i], buf);
    // printf("@%ld : %s\n", i, pBin);
    void *pMem = allocBlock(sizes[i], options[i], (int *)0);
    pBlocks[i] = pMem;

    blockInfo(pMem);
  }

  printf("\n---\n");

  for (size_t i = 0; i < numElements; i++)
  {
    freeBlock(pBlocks[i]);
  }

  memblockDestroy();

  return 0;
}
