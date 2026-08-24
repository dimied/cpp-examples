#ifndef LINUX_DL
#define LINUX_DL

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <features.h> //to make _GNU_SOURCE usable
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>

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

static int showInfo(struct dl_phdr_info *info, size_t size, void *data)
{
    int count = -1;

    if (data != 0)
    {
        count = *((int *)data);
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

        printf("    %2zu: [%14p; memsz:%7ld] flags: %#jx:%3s; ", j,
               realAddr,
               (uintmax_t)info->dlpi_phdr[j].p_memsz,
               (uintmax_t)info->dlpi_phdr[j].p_flags, flags);
        if (type != NULL)
            printf("%s\n", type);
        else
            printf("[other (%#x)]\n", p_type);
    }

    count++;

    if (data != 0)
    {
        *((int *)data) = count;
    }

    return 0;
}

int iterate()
{
    int data[4];
    int res = dl_iterate_phdr(showInfo, data);

    return res;
}

#endif