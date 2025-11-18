// memcpy.cpp : Defines the entry point for the console application.
//

#include <conio.h> // getch
#include <stdio.h>
#include <windows.h>


char *tbuf = NULL;

void memfill(void *dst, int n32, unsigned long i) {
  __asm {
		movq mm0, n32
		punpckldq mm0, mm0
		mov edi, dst

loopwrite:

		movntq 0[edi], mm0
		movntq 8[edi], mm0
        // movntq 16[edi], mm0
        // movntq 24[edi], mm0
        // movntq 32[edi], mm0
        // movntq 40[edi], mm0
        // movntq 48[edi], mm0
        // movntq 56[edi], mm0

		add edi, 16
		sub i, 2
		jg loopwrite

		emms
  }
}

void mem4(void *dst, void *src, int nbytes) {
  __asm {
        mov esi, src 
        mov ecx, nbytes 
        mov ebx, ecx 
        shr ebx, 11 // 2048 bytes at a time 
        mov edi, dst 

loop2k: // Copy 2k into temporary buffer 
        push edi 
        mov edi, tbuf 
        mov ecx, 2048 
        shr ecx, 6 

loopMemToL1: 
        prefetchnta 64[ESI] // Prefetch next loop, non-temporal 
        prefetchnta 96[ESI] 

        movq mm1,  0[ESI] // Read in source data 
        movq mm2,  8[ESI] 
        movq mm3, 16[ESI] 
        movq mm4, 24[ESI] 
        movq mm5, 32[ESI] 
        movq mm6, 40[ESI] 
        movq mm7, 48[ESI] 
        movq mm0, 56[ESI] 

        movq  0[EDI], mm1 // Store into L1 
        movq  8[EDI], mm2 
        movq 16[EDI], mm3 
        movq 24[EDI], mm4 
        movq 32[EDI], mm5 
        movq 40[EDI], mm6 
        movq 48[EDI], mm7 
        movq 56[EDI], mm0 
        add esi, 64 
        add edi, 64 
        dec ecx 
        jnz loopMemToL1 

        pop edi // Now copy from L1 to system memory 
        push esi 
        mov esi, tbuf 
        mov ecx, 2048 
        shr ecx, 6 

loopL1ToMem: 
        movq mm1, 0[ESI] // Read in source data from L1 
        movq mm2, 8[ESI] 
        movq mm3, 16[ESI] 
        movq mm4, 24[ESI] 
        movq mm5, 32[ESI] 
        movq mm6, 40[ESI] 
        movq mm7, 48[ESI] 
        movq mm0, 56[ESI] 

        movntq 0[EDI], mm1 // Non-temporal stores 
        movntq 8[EDI], mm2 
        movntq 16[EDI], mm3 
        movntq 24[EDI], mm4 
        movntq 32[EDI], mm5 
        movntq 40[EDI], mm6 
        movntq 48[EDI], mm7 
        movntq 56[EDI], mm0 

        add esi, 64 
        add edi, 64 
        dec ecx 
        jnz loopL1ToMem 

        pop esi // Do next 2k block 
        dec ebx 
        jnz loop2k 

		emms
  }
}

void mem3(void *dst, void *src, int nbytes) {
  _asm { 
        mov esi, src 
        mov edi, dst 
        mov ecx, nbytes 
        shr ecx, 6 // 64 bytes per iteration 

loop1: 
        prefetchnta 64[ESI] // Prefetch next loop, non-temporal 
        prefetchnta 96[ESI] 

        movq mm1,  0[ESI] // Read in source data 
        movq mm2,  8[ESI] 
        movq mm3, 16[ESI] 
        movq mm4, 24[ESI] 
        movq mm5, 32[ESI] 
        movq mm6, 40[ESI] 
        movq mm7, 48[ESI] 
        movq mm0, 56[ESI] 

        movntq  0[EDI], mm1 // Non-temporal stores 
        movntq  8[EDI], mm2 
        movntq 16[EDI], mm3 
        movntq 24[EDI], mm4 
        movntq 32[EDI], mm5 
        movntq 40[EDI], mm6 
        movntq 48[EDI], mm7 
        movntq 56[EDI], mm0 

        add esi, 64 
        add edi, 64 
        dec ecx 
        jnz loop1 

        emms
  }
}

void mem2(void *dst, void *src, int nbytes) {
  _asm { 
        mov esi, src 
        mov edi, dst 
        mov ecx, nbytes 
        shr ecx, 6 // 64 bytes per iteration 

loop1: 
        movq mm1,  0[ESI] // Read in source data 
        movq mm2,  8[ESI] 
        movq mm3, 16[ESI] 
        movq mm4, 24[ESI] 
        movq mm5, 32[ESI] 
        movq mm6, 40[ESI] 
        movq mm7, 48[ESI] 
        movq mm0, 56[ESI] 

        movntq  0[EDI], mm1 // Non-temporal stores 
        movntq  8[EDI], mm2 
        movntq 16[EDI], mm3 
        movntq 24[EDI], mm4 
        movntq 32[EDI], mm5 
        movntq 40[EDI], mm6 
        movntq 48[EDI], mm7 
        movntq 56[EDI], mm0 

        add esi, 64 
        add edi, 64 
        dec ecx 
        jnz loop1 

        emms
  }
}

void mem1(void *dst, void *src, int nbytes) {
  _asm { 
			mov esi, src 
			mov edi, dst 
			mov ecx, nbytes 
			shr ecx, 6 // 64 bytes per iteration 

	loop1: 
			movq mm1,  0[ESI] // Read in source data 
			movq mm2,  8[ESI] 
			movq mm3, 16[ESI] 
			movq mm4, 24[ESI] 
			movq mm5, 32[ESI] 
			movq mm6, 40[ESI] 
			movq mm7, 48[ESI] 
			movq mm0, 56[ESI] 

			movq  0[EDI], mm1 // Write to dstination 
			movq  8[EDI], mm2 
			movq 16[EDI], mm3 
			movq 24[EDI], mm4 
			movq 32[EDI], mm5 
			movq 40[EDI], mm6 
			movq 48[EDI], mm7 
			movq 56[EDI], mm0 

			add esi, 64 
			add edi, 64 
			dec ecx 
			jnz loop1 

			emms
  }
}

#define size 32768 * 1024

int main(int argc, char *argv[]) {
  char *foo = new char[size];
  char *foo2 = new char[size];
  // warm me up
  mem1(foo2, foo, size);
  mem1(foo2, foo, size);

  tbuf = new char[2048];

  LARGE_INTEGER s1, s2, f;
  ::QueryPerformanceFrequency(&f);

  double el;

  ::QueryPerformanceCounter(&s1);
  mem1(foo2, foo, size);
  ::QueryPerformanceCounter(&s2);
  el = s2.QuadPart - s1.QuadPart;
  el /= double(f.QuadPart);
  printf("SGI ex1: %fms = %fmb/sec\n", el * 1000,
         float(size) / 1024 / el / 1024);

  ::QueryPerformanceCounter(&s1);
  mem2(foo2, foo, size);
  ::QueryPerformanceCounter(&s2);
  el = s2.QuadPart - s1.QuadPart;
  el /= double(f.QuadPart);
  printf("SGI ex2: %fms = %fmb/sec\n", el * 1000,
         float(size) / 1024 / el / 1024);

  ::QueryPerformanceCounter(&s1);
  mem3(foo2, foo, size);
  ::QueryPerformanceCounter(&s2);
  el = s2.QuadPart - s1.QuadPart;
  el /= double(f.QuadPart);
  printf("SGI ex3: %fms = %fmb/sec\n", el * 1000,
         float(size) / 1024 / el / 1024);

  ::QueryPerformanceCounter(&s1);
  mem4(foo2, foo, size);
  ::QueryPerformanceCounter(&s2);
  el = s2.QuadPart - s1.QuadPart;
  el /= double(f.QuadPart);
  printf("SGI ex4: %fms = %fmb/sec\n", el * 1000,
         float(size) / 1024 / el / 1024);

  ::QueryPerformanceCounter(&s1);
  memcpy(foo2, foo, size);
  ::QueryPerformanceCounter(&s2);
  el = s2.QuadPart - s1.QuadPart;
  el /= double(f.QuadPart);
  printf("\nmemcpy %fms = %fmb/sec\n\n", el * 1000,
         float(size) / 1024 / el / 1024);

  for (int i = 0; i < 4; i++) {
    ::QueryPerformanceCounter(&s1);
    memfill(foo2, 0, size / 8);
    ::QueryPerformanceCounter(&s2);
    el = s2.QuadPart - s1.QuadPart;
    el /= double(f.QuadPart);
    printf("memfill %fms = %fmb/sec\n", el * 1000,
           float(size) / 1024 / el / 1024);
  }

  ::QueryPerformanceCounter(&s1);
  memset(foo2, 0, size);
  ::QueryPerformanceCounter(&s2);
  el = s2.QuadPart - s1.QuadPart;
  el /= double(f.QuadPart);
  printf("\nmemset %fms = %fmb/sec\n\n", el * 1000,
         float(size) / 1024 / el / 1024);

  printf("Press a key to exit...");
  _getch();

  return 0;
}
