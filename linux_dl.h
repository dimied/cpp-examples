#ifndef LINUX_DL
#define LINUX_DL

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <features.h> //to make _GNU_SOURCE usable
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <string.h>

const struct
{
    int type;
    char *name;
} typeMapping[] = {
    {PT_NULL, "unused"},
    // Loadable program segment
    {PT_LOAD, "PT_LOAD"},
    // Dynamic linking information
    {PT_DYNAMIC, "PT_DYNAMIC"},
    // Program interpreter
    {PT_INTERP, "PT_INTERP"},
    // Auxiliary information
    {PT_NOTE, "PT_NOTE"},
    // Reserved
    {PT_SHLIB, "PT_SHLIB"},
    // Entry for header table itself
    {PT_PHDR, "PT_PHDR"},
    // Thread-local storage segment
    {PT_TLS, "PT_TLS"},
    // Number of defined types
    {PT_NUM, "PT_NUM"},
    // GCC .eh_frame_hdr segment
    {PT_GNU_EH_FRAME, "PT_GNU_EH_FRAME"},
    // Indicates stack executability
    {PT_GNU_STACK, "PT_GNU_STACK"},
    // Read-only after relocation
    {PT_GNU_RELRO, "PT_GNU_RELRO"},
    // GNU property
    {PT_GNU_PROPERTY, "PT_GNU_PROPERTY"},
};

void showNote(void *data, uintmax_t memsize, uintmax_t alignment);
void showDynamic(void *data, uintmax_t size);

static int showInfo(struct dl_phdr_info *info, size_t size, void *data)
{
    int count = -1;

    if (data != 0)
    {
        count = *((int *)data);
    }

    char *name = strstr(info->dlpi_name, "libGL.so");
    if (name == NULL)
    {
        return 0;
    }

    printf("S:%d: %ld\n", count, size);

    printf("Name: \"%s\" (%d segments)\n", info->dlpi_name,
           info->dlpi_phnum);

    for (size_t j = 0; j < info->dlpi_phnum; j++)
    {
        int p_type = info->dlpi_phdr[j].p_type;
        char *type = NULL;

        for (size_t typeIdx = 0; typeIdx < sizeof(typeMapping) / sizeof(typeMapping[0]); typeIdx++)
        {
            if (p_type == typeMapping[typeIdx].type)
            {
                // Error: type = typeMapping->name;
                type = typeMapping[typeIdx].name;
                break;
            }
        }
        if (type == NULL && (p_type >= PT_LOOS) && (p_type <= PT_HIOS))
        {
            type = "OS specific";
        }
        char flags[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        int fIdx = 0;
        uintmax_t p_flags = (uintmax_t)info->dlpi_phdr[j].p_flags;

        if (p_flags & PF_R)
        {
            flags[0] = 'R';
            fIdx++;
        }
        if (p_flags & PF_W)
        {
            flags[fIdx] = 'W';
            fIdx++;
        }
        if (p_flags & PF_X)
        {
            flags[fIdx] = 'X';
            fIdx++;
        }
        void *realAddr = (void *)(info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);

        printf("  %2zu: [%14p; memsz:%7ld; align: %4ld] flags: %#jx:%3s; ", j,
               realAddr,
               (uintmax_t)info->dlpi_phdr[j].p_memsz,
               (uintmax_t)info->dlpi_phdr[j].p_align,
               (uintmax_t)info->dlpi_phdr[j].p_flags, flags);

        if (type != NULL)
        {
            printf("%s\n", type);
        }
        else
        {
            printf("[other (%#x)]\n", p_type);
        }
#if 0
        if (p_type == PT_NOTE && (p_flags & PF_R) != 0)
        {
            showNote(realAddr, (uintmax_t)info->dlpi_phdr[j].p_memsz,
                     4);
        }
#endif
        if (p_type == PT_LOAD && (p_flags & PF_X) && (p_flags & PF_R))
        {
            printf("DYNAMIC:\n");
            //showDynamic(realAddr, (uintmax_t)info->dlpi_phdr[j].p_memsz);
            printf("---\n");
        }
    }

    count++;

    if (data != 0)
    {
        *((int *)data) = count;
    }

    return 0;
}

void showDynamic(void *base, uintmax_t size)
{
    Dl_info info;
    char *dst = (char *)base;

    uintmax_t s = 0;
    const size_t offset = 8;

    while (s + offset <= size)
    {
        const void *addr = dst;
        dst += offset;
        s += offset;
        int res = dladdr(addr, &info);

        printf("RES: %d\n", res);

        if (res != 0)
        {
             printf("dli_fname: %s\n", info.dli_fname);
            // printf("dli_fbase: %p\n", info.dli_fbase);
            printf("dli_sname: %s\n", info.dli_sname);
            printf("dli_saddr: %p\n", info.dli_saddr);
        }
        else
        {
            printf("FAILED\n");
            return;
        }
    }
}

int iterate()
{
    int data[4];
    int res = dl_iterate_phdr(showInfo, data);

    return res;
}

void showNote(void *data, uintmax_t memsize, uintmax_t alignment)
{
    // structure, aligned to alignment
    // namesz
    // descsz
    // type
    // name (if namesz>0)
    // desc (if descsz>0)
    // repeating: ...

    printf("Note: sz=%ld | align=%ld\n", memsize, alignment);

    if ((alignment == 4) || (alignment >= 8))
    {
        // namesz, descsz, type
        uintmax_t minSize = 12;
        if (alignment >= 8)
        {
            minSize = 24;
        }

        if (memsize < minSize)
        {
            printf("Size error!\n");
            return;
        }
        // memsize -= minSize;

        char *cData = (char *)data;
        char *curData = cData;
        long int sizeLeft = (long int)memsize;

        while (curData - cData <= (long int)memsize)
        {
            char *name, *desc;
            uint64_t szName = 0, szDesc = 0, type = 0;
            size_t step = 4;

            if (alignment == 4)
            {
                szName = *((uint32_t *)curData);
                curData += 4;
                szDesc = *((uint32_t *)curData);
                curData += 4;
                type = *((uint32_t *)curData);
                curData += 4;
                sizeLeft -= 12;
            }
            else
            {
                step = 8;
                szName = *((uint64_t *)curData);
                curData += 8;
                szDesc = *((uint64_t *)curData);
                curData += 8;
                type = *((uint64_t *)curData);
                curData += 8;
                sizeLeft -= 24;
            }
            if (sizeLeft <= 0)
            {
                return;
            }

            if ((szName > INT32_MAX) || (szDesc >= INT32_MAX))
            {
                return;
            }

            if (szName > 0)
            {
                if ((long int)szName > sizeLeft)
                {
                    return;
                }
                size_t s = (szName + step) & (~step);
                name = curData;
                curData += s;
            }
            if (szDesc > 0)
            {
                if ((long int)szDesc > sizeLeft)
                {
                    return;
                }
                size_t s = (szDesc + step) & (~step);
                desc = curData;
                curData += s;
            }
            printf("Name: %ld | %s\n", szName, name);
            printf("Desc: %ld | %s\n", szDesc, desc);
            printf("Type: %ld\n", type);
        }
    }
    else
    {
        printf("Alignment error!\n");
    }
}

//
void findModule()
{
    Dl_info info;
    char *pError;
    const void *addr = (void *)glGetError;

    int res = dladdr(addr, &info);

    void *handle = NULL;
    char fPath[128];
    memset(fPath, 0, 128);

    printf("RES: %d\n", res);

    if (res != 0)
    {
        if (info.dli_fname != 0)
        {
            size_t s = strlen(info.dli_fname);
            if (s < 128)
            {
                strncpy(fPath, info.dli_fname, s);
            }
        }
        printf("dli_fname: %s\n", info.dli_fname);
        printf("dli_fbase: %p\n", info.dli_fbase);
        printf("dli_sname: %s\n", info.dli_sname);
        printf("dli_saddr: %p\n", info.dli_saddr);
    }
    else
    {
        pError = dlerror();
        printf("ERROR:%s\n", pError);
        return;
    }
    if (fPath[0] == 0)
    {
        return;
    }
    handle = dlopen(fPath, RTLD_NOW);

    if (handle == NULL)
    {
        pError = dlerror();
        printf("ERROR:dlopen:%s\n", pError);
        return;
    }

    printf("Loaded: %s\n", fPath);

    char *symbols[] = {"glGetString", "glBegin"};

    for (size_t i = 0; i < 2; i++)
    {
        void *sr = dlsym(handle, symbols[i]);
        printf("S: %s | %p\n", symbols[i], sr);
    }
}

#endif