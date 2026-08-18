
#include <stdio.h>

unsigned int registers[128];
unsigned long int ulRegisters[128];

unsigned char steps[128] = {0, 3, 3, 2, 3, 0};
unsigned char usage[128] = {0, 7, 7, 7, 7, 7, 7, 0};
unsigned char funcTypes[128];

void *funcPtrs[128];

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

int neg(int a)
{
    return -a;
}

int sum(int *pData, int size)
{
    int result = 0;
    // printf("Call sum: %p | %d\n", pData, size);
    for (int i = 0; i < size; i++)
    {
        result += pData[i];
    }
    return result;
}

unsigned char *curCmds = (unsigned char *)0;

typedef union _MultiPtr
{
    unsigned int *uip;
    int *ip;
    float *fp;
    void *vp;

} MultiPtr;

int* ptr1() {
    return (int*)&registers[curCmds[1]];
}

int iDump[4];
unsigned int uiDump[4];

int execute(unsigned char *cmds, int size)
{
    // printf("IN: %d\n", size);

    for (int i = 0; i < size; i++)
    {
        // printf("Check: %d\n", i);

        unsigned char c = cmds[i];
        const int s = steps[c];

        if (s == 0)
        {
            return 2;
        }

        const void *p = funcPtrs[c];
#if ALLOW_FULL_VOID_FUNCTIONS
        if (s > 250)
        {
            void (*voidFuncPtr)() = p;
            voidFuncPtr();
            continue;
        }
#endif

        if (i + s > size)
        {
            return 3;
            // printf("OoR: %d | Size: %d, i = %d\n", s, size, i);
            // break;
        }

        // get dynamically
        const unsigned char usedPointers = usage[c];
        //curCmds = &cmds[i];

        unsigned char c1 = cmds[i + 1];
        int *ip1 = iDump;
        int *ip2 = iDump, *ip3 = iDump;

        if (usedPointers & 0x1)
        {
            ip1 = (int *)&registers[c1];
        }

#if USE_UIPTR
        unsigned int *uip1;

        if (usedPointers & 0x2)
        {
            uip1 = &registers[c1];
        }
#endif
#if USE_FPTR
        float *fp1;
        if (usedPointers & 0x4)
        {
            fp1 = (float *)((void *)&registers[c1]);
        }
#endif

        unsigned char c2, c3;
        
#if USE_UIPTR
        unsigned int *uip2, *uip3;
#endif
#if USE_FPTR
        float *fp2, *fp3;
#endif

        // MultiPtr mp1;
        // mp1.uip = &registers[c1];

        if (s > 1)
        {
            c2 = cmds[i + 2];
            if (usedPointers & 0x1)
            {
                ip2 = (int *)&registers[c2];
            }
#if USE_UIPTR
            if (usedPointers & 0x2)
            {
                uip2 = &registers[c2];
            }
#endif
#if USE_FPTR
            if (usedPointers & 0x4)
            {
                fp2 = (float *)((void *)&registers[c2]);
            }
#endif

if (s > 2)
        {
            c3 = cmds[i + 3];
            if (usedPointers & 0x1)
            {
                ip3 = (int *)&registers[c3];
            }
#if USE_UIPTR
            if (usedPointers & 0x2)
            {
                uip3 = &registers[c3];
            }
#endif
#if USE_FPTR
            if (usedPointers & 0x4)
            {
                fp3 = (float *)((void *)&registers[c3]);
            }
#endif
        }
        }

        

        // printf("Step:%d: %d\n", c, s);

        c = funcTypes[c];

        switch (c)
        {
        case 1:
            int (*fptr1)(int a, int b) = p;
            //ip1 = ptr1();
            // p1 = (int *)&registers[c1];
            // p2 = (int *)&registers[c2];
            // p3 = (int *)&registers[c3];
            *ip3 = fptr1(*ip1, *ip2);
            // printf("P:1 %d | P2: %d | P3: %d\n", *p1, *p2, *p3);
            break;
        case 2:
            int (*fptr2)(int a) = p;
            // p1 = (int *)&registers[c1];
            // p2 = (int *)&registers[c2];
            *ip2 = fptr2(*ip1);
            // printf("P:1 %d | P2: %d\n", *p1, *p2);
            break;
        case 3:
            int (*fptr3)(int *a, int b) = p;
            // p1 = (int *)&registers[c1];
            // p2 = (int *)&registers[c2];
            // p3 = (int *)&registers[c3];
            // int v = *p2;
            *ip3 = fptr3(ip1, *ip2);
            // printf("CALL:3: %p(%d) | %d(%d) = %d(%d)\n", p1,c1,  v,c2, *p3, c3);
            break;
        default:
            // printf("Unknown cmds @%d\n", i);
            return 1;
        }
        i += s;
    }
    return 0;
}

int main()
{
    funcPtrs[1] = (void *)&add;
    funcPtrs[2] = (void *)&sub;
    funcPtrs[3] = (void *)&neg;
    funcPtrs[4] = (void *)&sum;
    funcTypes[1] = 1;
    funcTypes[2] = 1;
    funcTypes[3] = 2;
    funcTypes[4] = 3;

    registers[0] = 10;
    registers[2] = 12;

    registers[3] = 80;
    registers[10] = 10;
    registers[20] = 12;

    unsigned char cmds[15] = {1, 0, 2, 3,
                              2, 0, 2, 4,
                              3, 3, 5,
                              4, 0, 20, 36};

    int *iregisters = (int *)&registers[0];

    execute(cmds, (int)sizeof(cmds));

    printf("add(%d, %d)= %d\n", iregisters[0], iregisters[2], iregisters[3]);
    printf("sub(%d, %d)= %d\n", iregisters[0], iregisters[2], iregisters[4]);
    printf("neg(%d)= %d\n", iregisters[3], iregisters[5]);
    printf("sum(...)= %d\n", iregisters[36]);
    // printf("sum:impl= %d\n", sum(iregisters, 12));

    return 0;
}
