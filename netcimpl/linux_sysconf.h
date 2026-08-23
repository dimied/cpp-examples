#ifndef LINUX_SYS_CONF_DATA
#define LINUX_SYS_CONF_DATA

#include <unistd.h>
#include <sys/sysinfo.h>

#define LINUX_SYS_CONF_DATA_USE_ALL (0xFFFFFFFul)

#define LINUX_SYS_CONF_DATA_USE_NUM_CPUS (0x1)
#define LINUX_SYS_CONF_DATA_USE_MAX_PROCESSES (0x2)
#define LINUX_SYS_CONF_DATA_USE_MAX_OPEN_FILES (0x4)
#define LINUX_SYS_CONF_DATA_USE_MAX_STREAMS (0x8)

#define LINUX_SYS_CONF_DATA_USE_PAGE_SIZE (0x10)
#define LINUX_SYS_CONF_DATA_USE_PHYSICAL_PAGES (0x20)
#define LINUX_SYS_CONF_DATA_USE_PHYSICAL_PAGES_AVAILABLE (0x40)

#define LINUX_SYS_CONF_DATA_USE_CUSTOM_FIELD (0x1000000000)

#define LINUX_SYS_CONF_DATA_CUSTOM_FIELD_TYPE unsigned int
#define LINUX_SYS_CONF_DATA_CUSTOM_FIELD_NAME type

#ifndef LINUX_SYS_CONF_DATA_MY_USE
#define LINUX_SYS_CONF_DATA_MY_USE LINUX_SYS_CONF_DATA_USE_ALL
#endif

typedef struct SysConfDataCacheInfoType
{
    long int dataCacheLinesize;
    long int instructionCacheLinesize;
    long int dataCacheSize;
    long int instructionCacheSize;
    long int dataCacheAssoc;
    long int instructionCacheAssoc;
    long int type;
} SysConfDataCacheInfo;

unsigned int cache1Props[] = {
    _SC_LEVEL1_DCACHE_LINESIZE,
    _SC_LEVEL1_ICACHE_LINESIZE,
    _SC_LEVEL1_DCACHE_SIZE,
    _SC_LEVEL1_ICACHE_SIZE,
    _SC_LEVEL1_DCACHE_ASSOC,
    _SC_LEVEL1_ICACHE_ASSOC,
};

unsigned int cache2Props[] = {
    _SC_LEVEL2_CACHE_LINESIZE,
    _SC_LEVEL2_CACHE_LINESIZE,
    _SC_LEVEL2_CACHE_SIZE,
    _SC_LEVEL2_CACHE_SIZE,
    _SC_LEVEL2_CACHE_ASSOC,
    _SC_LEVEL2_CACHE_ASSOC,
};

unsigned int cache3Props[] = {
    _SC_LEVEL3_CACHE_LINESIZE,
    _SC_LEVEL3_CACHE_LINESIZE,
    _SC_LEVEL3_CACHE_SIZE,
    _SC_LEVEL3_CACHE_SIZE,
    _SC_LEVEL3_CACHE_ASSOC,
    _SC_LEVEL3_CACHE_ASSOC,
};

unsigned int cache4Props[] = {
    _SC_LEVEL4_CACHE_LINESIZE,
    _SC_LEVEL4_CACHE_LINESIZE,
    _SC_LEVEL4_CACHE_SIZE,
    _SC_LEVEL4_CACHE_SIZE,
    _SC_LEVEL4_CACHE_ASSOC,
    _SC_LEVEL4_CACHE_ASSOC,
};

typedef struct SysConfDataType
{
#if (LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_CUSTOM_FIELD)
    LINUX_SYS_CONF_DATA_CUSTOM_FIELD_TYPE LINUX_SYS_CONF_DATA_CUSTOM_FIELD_NAME;
#endif
    /**
     * Given options for retrieval
     */
    unsigned int options;
    /**
     * Stores retrieval statuses
     */
    unsigned int statuses;
    unsigned int cpu;

#if (LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_NUM_CPUS)
    size_t numCpus;
#endif

#if (LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_MAX_PROCESSES)
    size_t numMaxProcesses;
#endif

#if (LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_MAX_OPEN_FILES)
    size_t numMaxOpenFiles;
#endif

#if (LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_MAX_STREAMS)
    size_t maxStreams;
#endif
    size_t maxThreads;

#if (LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_PAGE_SIZE)
    size_t pageSize;
#endif

#if (LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_PHYSICAL_PAGES)
    size_t physicalPages;
#endif

#if (LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_PHYSICAL_PAGES_AVAILABLE)
    size_t physicalPagesAvailable;
#endif

    size_t numCaches;
    SysConfDataCacheInfo caches[4];

    unsigned int support;

    size_t hostnameMaxLength;
    struct SysConfDataType *next;
} SysConfData;

void setCacheInfo(SysConfDataCacheInfo *pData, unsigned int *props)
{
    pData->dataCacheLinesize = sysconf(props[0]);
    pData->instructionCacheLinesize = sysconf(props[1]);
    pData->dataCacheSize = sysconf(props[2]);
    pData->instructionCacheSize = sysconf(props[3]);
    pData->dataCacheAssoc = sysconf(props[4]);
    pData->instructionCacheAssoc = sysconf(props[5]);
}

int copySysConf(SysConfData *pResult, SysConfData *pSource)
{
    // TODO
    return 1;
}

void freeSysConf(SysConfData *pResult)
{
    if (pResult != 0)
    {
        free(pResult);
    }
}

#define SUPPORTS_REAL_TIME_TIMER 0x1
#define SUPPORTS_ASYNC_IO 0x2
#define SUPPORTS_SYNC_IO 0x4
#define SUPPORTS_FSYNC 0x8

#define SUPPORTS_MEMORY_MAPPED_FILES 0x10
#define SUPPORTS_MEMLOCK 0x20
#define SUPPORTS_MEMLOCK_RANGE 0x40

#define SUPPORTS_SEMAPHORES 0x100
#define SUPPORTS_BARRIERS 0x200
#define SUPPORTS_SHARED_MEMORY_OBJECTS 0x400
#define SUPPORTS_REALTIME_SIGNALS 0x800

#define SUPPORTS_MESSAGE_PASSING 0x1000

char *supportNames[] = {
    "Timers",
    "AsyncIO",
    "SyncIO",
    "Filesync",

    "Mapped files",
    "Memlock",
    "Memlock-range",
    "",
    "Semaphores",
    "Sync barriers",
    "Shared mem objects",
    "Signals(RT)",

    "Message passing",
};

unsigned int supportProps[][2] = {
    {_SC_TIMERS, SUPPORTS_REAL_TIME_TIMER},
    {_SC_ASYNCHRONOUS_IO, SUPPORTS_ASYNC_IO},
    {_SC_SYNCHRONIZED_IO, SUPPORTS_SYNC_IO},
    {_SC_FSYNC, SUPPORTS_FSYNC},

    {_SC_MAPPED_FILES, SUPPORTS_MEMORY_MAPPED_FILES},
    {_SC_MEMLOCK, SUPPORTS_MEMLOCK},
    {_SC_MEMLOCK_RANGE, SUPPORTS_MEMLOCK_RANGE},

    {_SC_SEMAPHORES, SUPPORTS_SEMAPHORES},
    {_SC_BARRIERS, SUPPORTS_BARRIERS},

    {_SC_SHARED_MEMORY_OBJECTS, SUPPORTS_SHARED_MEMORY_OBJECTS},
    {_SC_REALTIME_SIGNALS, SUPPORTS_REALTIME_SIGNALS},

    {_SC_MESSAGE_PASSING, SUPPORTS_MESSAGE_PASSING},
    //
    {_SC_TIMERS, 0x1}};

SysConfData *getSysConf(int options, SysConfData *pResult)
{
    if (pResult == 0)
    {
        pResult = (SysConfData *)calloc(1, sizeof(SysConfData));
    }

    if (pResult != 0)
    {
        pResult->numCpus = sysconf(_SC_NPROCESSORS_CONF);
        pResult->numMaxProcesses = sysconf(_SC_CHILD_MAX);
        pResult->numMaxOpenFiles = sysconf(_SC_OPEN_MAX);
        pResult->maxStreams = sysconf(_SC_STREAM_MAX);

        pResult->maxThreads = sysconf(_SC_THREAD_THREADS_MAX);

        pResult->pageSize = sysconf(_SC_PAGESIZE);
        pResult->physicalPages = sysconf(_SC_PHYS_PAGES);
        pResult->physicalPagesAvailable = sysconf(_SC_AVPHYS_PAGES);

        /*
                _SC_THREAD_STACK_MIN
        */

        unsigned int support = 0;

        long int res = 0;

        for (int i = 0; i < sizeof(supportProps) / sizeof(supportProps[0]); i++)
        {
            res = sysconf(supportProps[i][0]);
            if (res > 0)
            {
                support |= supportProps[i][1];
            }
        }

        pResult->support = support;

        setCacheInfo(&pResult->caches[0], cache1Props);
        setCacheInfo(&pResult->caches[1], cache2Props);
        setCacheInfo(&pResult->caches[2], cache3Props);

        pResult->hostnameMaxLength = sysconf(_SC_HOST_NAME_MAX);
    }
    return pResult;
}

int parseSysData();

int printSysData(SysConfData *pData, char *pOutput, size_t outputLength)
{
    if (pData == 0)
    {
        return 1;
    }
    // char buffer[100];
    // memset(buffer, 0, 100);

    printf("CPUs: %ld\n", pData->numCpus);
    printf("Max child processes: %ld\n", pData->numMaxProcesses);
    printf("Max open files: %ld\n", pData->numMaxOpenFiles);
    printf("Max streams: %ld\n", pData->maxStreams);
    printf("Max threads: %ld\n", pData->maxThreads);

    printf("Page size(in bytes): %ld\n", pData->pageSize);
    printf("Physical pages: %ld\n", pData->physicalPages);
    printf("Physical pages(available): %ld\n", pData->physicalPagesAvailable);

    for (int i = 10; i < 4; i++)
    {
        printf("Cache:%d:data:linesize: %ld\n", i, pData->caches[i].dataCacheLinesize);
        printf("Cache:%d:inst:linesize: %ld\n", i, pData->caches[i].instructionCacheLinesize);
        printf("Cache:%d:data:size: %ld\n", i, pData->caches[i].dataCacheSize);
        printf("Cache:%d:inst:size: %ld\n", i, pData->caches[i].instructionCacheSize);
        printf("Cache:%d:data:assoc: %ld\n", i, pData->caches[i].dataCacheAssoc);
        printf("Cache:%d:inst:assoc: %ld\n", i, pData->caches[i].instructionCacheAssoc);
    }

    unsigned int support = pData->support;
    printf("Support: %x\n", support);

    size_t numAllStrings = sizeof(supportNames) / sizeof(supportNames[0]);
    char *unknown = "?";

    for (size_t i = 0; i < 16; i++)
    {
        size_t val = (support >> i) & 0x1;
        char *s = (i < numAllStrings) ? supportNames[i] : unknown;
        if (val)
        {
            printf("\t%s: %lx\n", s, val);
        }
    }

    struct sysinfo si;

    int siRes = sysinfo(&si);
    if (siRes != 0)
    {
        printf("sysinfo: %d\n", siRes);
    }
    else
    {
        printf("MU: %d\n", si.mem_unit);
    }

    printf("Hostname max length: %ld\n", pData->hostnameMaxLength);
    printf("Struct size:: %ld\n", sizeof(*pData));
    // TODO: https://en.wikipedia.org/wiki/CPUID
    parseSysData();

    return 0;
}

#include <dirent.h>

int parseSysData()
{

    unsigned int i = 0;
// #if defined(linux)
#ifdef __unix__

    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir("/sys/devices/system/cpu/");

    if (dr == NULL) // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory");
        return 1;
    }

    struct dirent *de; // for directory entry

    SysConfData data[64];
    memset(data, 0, sizeof(data));

    int cpu = -1, cur;

    unsigned long int assignedCPUs = 0;
    char *name;
    size_t nameLen;

    while ((de = readdir(dr)) != NULL)
    {
        name = de->d_name;
        nameLen = strlen(name);

        if (nameLen > 3 && (0 == strncmp("cpu", name, 3)))
        {
            if (1 == sscanf(name, "cpu%d", &cur))
            {
                if (cpu < cur)
                {
                    cpu = cur;
                }
                printf("D: %s\n", name);
                if (cur < 64)
                {
                    assignedCPUs |= (0x1UL) << cur;
                }
            }
        }
    }
    closedir(dr);
    char fullPathBuffer[128];

#define NUM_NAMES 5
    char *names[NUM_NAMES] = {
        "level",
        "coherency_line_size",
        "shared_cpu_map",
        "size",
        "type"};
    char *cacheTypes[] = {"Data", "Instruction", "Unified"};

    char valueCache[32];

    for (int i = 0; i < cpu; i++)
    {
        memset(fullPathBuffer, 0, 128);
        snprintf(fullPathBuffer, 127, "/sys/devices/system/cpu/cpu%d/cache/", i);
        dr = opendir(fullPathBuffer);

        if (dr != 0)
        {
            while ((de = readdir(dr)) != NULL)
            {
                name = de->d_name;
                nameLen = strlen(name);

                if (nameLen > 5 && (0 == strncmp("index", name, 5)))
                {
                    snprintf(fullPathBuffer, 127, "/sys/devices/system/cpu/cpu%d/cache/%s/", i, name);

                    DIR *indexDIR = opendir(fullPathBuffer);
                    if (indexDIR != 0)
                    {
                        printf("D: %s\n", fullPathBuffer);
                        struct dirent *indexDE;

                        int level = 1;
                        int cacheline = 0;
                        unsigned long int cpuMap = 0;
                        unsigned long int value = 0;
                        unsigned int valueUnit = 0;
                        int cacheType = -1;

                        while ((indexDE = readdir(indexDIR)) != NULL)
                        {
                            char *fileName = indexDE->d_name;
                            //printf("\t%s\n", fileName);

                            int hit = -1;

                            for (int j = 0; j < NUM_NAMES; j++)
                            {
                                if (strcmp(fileName, names[j]) == 0)
                                {
                                    hit = j;
                                    break;
                                }
                            }

                            if (hit != -1)
                            {
                                //printf("HIT: %s\n", fileName);

                                snprintf(fullPathBuffer, 127, "/sys/devices/system/cpu/cpu%d/cache/%s/%s", i, name, fileName);

                                FILE *p = fopen(fullPathBuffer, "r");

                                if (p)
                                {
                                    switch (hit)
                                    {
                                    case 0:
                                        // level
                                        fscanf(p, "%d", &level);
                                        break;
                                    case 1:
                                        // cacheline
                                        fscanf(p, "%d", &cacheline);
                                        break;
                                    case 2:
                                        // shared_map
                                        fscanf(p, "%lx", &cpuMap);
                                        break;
                                    case 3:
                                        // size
                                        memset(valueCache, 0, sizeof(valueCache));
                                        fscanf(p, "%s", valueCache);
                                        if (1 == sscanf(valueCache, "%ldK", &value))
                                        {
                                            valueUnit = 10;
                                        }
                                        else if (1 == sscanf(valueCache, "%ldM", &value))
                                        {
                                            valueUnit = 20;
                                        }
                                        break;
                                    case 4:
                                        // type
                                        memset(valueCache, 0, sizeof(valueCache));
                                        fscanf(p, "%s", valueCache);
                                        for (int ci = 0; ci < 3; ci++)
                                        {
                                            if (strcmp(valueCache, cacheTypes[ci]) == 0)
                                            {
                                                cacheType = ci;
                                                break;
                                            }
                                        }
                                        break;
                                    }

                                    fclose(p);
                                }
                            }
                        }

                        //

                        printf("L:%d| T:%d| LS:%d| CPUS:%lx| SZ:%ld| SZU: %d\n",
                               level, cacheType, cacheline, cpuMap, value, valueUnit);
                        closedir(indexDIR);
                    }
                }
            }
            closedir(dr);
        }
    }

    printf("Assigned: %lx\n", assignedCPUs);

    FILE *p = 0;
    p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");

    if (p)
    {
        fscanf(p, "%d", &i);
        fclose(p);
    }

    printf("CLS: %d\n", i);
    return 0;
#else
#error Unrecognized platform
#endif
    return 1;
}

/*
// Source - https://stackoverflow.com/a/4049562
// Posted by Nick Strupat, modified by community. See post 'Timeline' for change history
// Retrieved 2026-08-22, License - CC BY-SA 3.0

#ifndef GET_CACHE_LINE_SIZE_H_INCLUDED
#define GET_CACHE_LINE_SIZE_H_INCLUDED

// Author: Nick Strupat
// Date: October 29, 2010
// Returns the cache line size (in bytes) of the processor, or 0 on failure

#include <stddef.h>
size_t cache_line_size();

#if defined(__APPLE__)

#include <sys/sysctl.h>
size_t cache_line_size()
{
    size_t line_size = 0;
    size_t sizeof_line_size = sizeof(line_size);
    sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, 0, 0);
    return line_size;
}

#elif defined(_WIN32)

#include <stdlib.h>
#include <windows.h>
size_t cache_line_size()
{
    size_t line_size = 0;
    DWORD buffer_size = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buffer = 0;

    GetLogicalProcessorInformation(0, &buffer_size);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
    GetLogicalProcessorInformation(&buffer[0], &buffer_size);

    for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i)
    {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1)
        {
            line_size = buffer[i].Cache.LineSize;
            break;
        }
    }

    free(buffer);
    return line_size;
}
*/

#endif