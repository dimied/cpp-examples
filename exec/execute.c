// x86/x64 Runtime Code Generation Demonstration (Linux/macOS/Windows)
// Copyright (C) 2017 Jeffrey L. Overbey.

#if !defined(_WIN32) && !defined(_WIN64) ////////////////////////// Linux/macOS

#include <stddef.h>    // size_t
#include <stdint.h>    // uint8_t, uint32_t
#include <math.h>
#include <stdio.h>     // printf
#include <string.h>    // memcpy
#include <sys/mman.h>  // mmap, mprotect, munmap, MAP_FAILURE

// Machine code for "mov eax, 12345678h" followed by "ret"
uint8_t machine_code[] = { 0xB8, 0x78, 0x78, 0x34, 0x12, 0xC3 };

float r[3] = {1,2,3};

void* allocate_and_copy(void *data, uint32_t size) {
    void *mem = mmap(NULL, sizeof(machine_code), PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (mem == MAP_FAILED) {
        perror("mmap");
        return 0;
    }

    memcpy(mem, machine_code, sizeof(machine_code));

    // Change the page protections to read+execute
    if (mprotect(mem, sizeof(machine_code), PROT_READ | PROT_EXEC) == -1) {
        perror("mprotect");
        return 0;
    }
    return mem;
}

int deallocate(void* execData, uint32_t size) {
    return (munmap(execData, size) == -1) ? 1 : 0;
}

void f(float u, float v, float* pResult) {
    *pResult = u+v;
    pResult++;
    *pResult = u*v;
    pResult++;
    *pResult = (float)(cosf(u) * sinf(v));
}

int main(int argc, char **argv) {

    void* mem = allocate_and_copy(machine_code, (uint32_t)sizeof(machine_code));

    if(mem == 0) {
        return 1;
    }

    // Point a function pointer at the newly allocated page, then call it
    uint32_t(*fn)() = (uint32_t(*)()) mem;
    uint32_t result = fn();
    //printf("Execute ...\n");
    printf("result = 0x%x\n", result);
    

    f(1, 2, r);
    printf("1: %.2lf\n", r[0]);
    printf("2: %.2lf\n", r[1]);
    printf("3: %.2lf\n", r[2]);

    return deallocate(mem, (uint32_t)sizeof(machine_code));
}

#else // defined(_WIN32) || defined(_WIN64) /////////////////////////// Windows

#include <stddef.h>    // size_t
#include <stdint.h>    // uint8_t, uint32_t
#include <stdio.h>     // printf
#include <memory.h>    // memcpy_s
#include <tchar.h>     // Must be included before strsafe.h
#include <strsafe.h>
#include <windows.h>

// Display the error message corresponding to GetLastError() in a message box.
static void DisplayError(LPTSTR failedFunctionName);

uint8_t machine_code[] = { 0xB8, 0x78, 0x56, 0x34, 0x12, 0xC3 };

int _tmain(int argc, _TCHAR **argv) {
    // Allocate a new page of memory, setting its protections to read+write
    LPVOID mem = VirtualAlloc(NULL, sizeof(machine_code),
        MEM_COMMIT, PAGE_READWRITE);
    if (mem == NULL) {
        DisplayError(TEXT("VirtualAlloc"));
        return 1;
    }

    // Write the machine code into the newly allocated page
    if (memcpy_s(mem, sizeof(machine_code), machine_code, sizeof(machine_code))) {
        DisplayError(TEXT("memcpy_s"));
        return 2;
    }

    // Change the page protections to read+execute
    DWORD ignore;
    if (!VirtualProtect(mem, sizeof(machine_code), PAGE_EXECUTE_READ, &ignore)) {
        DisplayError(TEXT("VirtualAlloc"));
        return 3;
    }

    // Flush the instruction cache
    if (!FlushInstructionCache(GetCurrentProcess(), mem, sizeof(machine_code))) {
        DisplayError(TEXT("FlushInstructionCache"));
        return 4;
    }

    // Point a function pointer at the newly allocated page, then call it
    uint32_t(*fn)() = (uint32_t(*)()) mem;
    uint32_t result = fn();
    _tprintf(TEXT("result = 0x%x\n"), result);

    // Free the memory
    if (!VirtualFree(mem, 0, MEM_RELEASE)) {
        DisplayError(TEXT("VirtualFree"));
        return 5;
    }

    return 0;
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/ms680582.aspx
static void DisplayError(LPTSTR failedFunctionName) {
    DWORD errorCode = GetLastError();
    LPVOID msgBufPtr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_FROM_SYSTEM
                      | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  errorCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&msgBufPtr,
                  0,
                  NULL);

    size_t size = sizeof(TCHAR) * (lstrlen((LPCTSTR)msgBufPtr)
                                   + lstrlen((LPCTSTR)failedFunctionName)
                                   + 40 /* Static text below */);
    LPVOID displayBufPtr = (LPVOID)LocalAlloc(LMEM_ZEROINIT, size);
    StringCchPrintf((LPTSTR)displayBufPtr,
                    LocalSize(displayBufPtr) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"),
                    failedFunctionName,
                    errorCode,
                    msgBufPtr);
    MessageBox(NULL, (LPCTSTR)displayBufPtr, TEXT("Error"), MB_ICONERROR);

    LocalFree(msgBufPtr);
    LocalFree(displayBufPtr);
}

#endif
