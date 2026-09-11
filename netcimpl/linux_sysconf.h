#ifndef LINUX_SYS_CONF_DATA
#define LINUX_SYS_CONF_DATA

#include <unistd.h>
#include <sys/sysinfo.h>
#include <errno.h>

#define LINUX_SYS_CONF_DATA_USE_ALL (0xFFFFFFFul)

#define LINUX_SYS_CONF_DATA_USE_NUM_CPUS_SHIFT 0
#define LINUX_SYS_CONF_DATA_USE_MAX_PROCESSES_SHIFT 1
#define LINUX_SYS_CONF_DATA_USE_MAX_OPEN_FILES_SHIFT 2

#define LINUX_SYS_CONF_DATA_USE_NUM_CPUS (0x1)
#define LINUX_SYS_CONF_DATA_USE_MAX_PROCESSES (0x1 << LINUX_SYS_CONF_DATA_USE_MAX_PROCESSES_SHIFT)
#define LINUX_SYS_CONF_DATA_USE_MAX_OPEN_FILES (0x1 << LINUX_SYS_CONF_DATA_USE_MAX_OPEN_FILES_SHIFT)
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
     * Check using IS_STATUS_OK(pResult, BITMASK)
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
    if ((pResult == 0) || (pSource == 0))
    {
        return 1;
    }
    return 0;
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

#include <stddef.h>

unsigned char offsets[][3] = {
#if LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_NUM_CPUS
    {LINUX_SYS_CONF_DATA_USE_NUM_CPUS, _SC_NPROCESSORS_CONF, offsetof(SysConfData, numCpus)},
#endif
#if LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_MAX_PROCESSES
    {LINUX_SYS_CONF_DATA_USE_MAX_PROCESSES, _SC_CHILD_MAX, offsetof(SysConfData, numMaxProcesses)},
#endif
#if LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_MAX_OPEN_FILES
    {LINUX_SYS_CONF_DATA_USE_MAX_OPEN_FILES, _SC_OPEN_MAX, offsetof(SysConfData, numMaxOpenFiles)},
#endif
#if LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_MAX_STREAMS
    {LINUX_SYS_CONF_DATA_USE_MAX_STREAMS, _SC_STREAM_MAX, offsetof(SysConfData, maxStreams)},
#endif
#if LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_PAGE_SIZE
    {LINUX_SYS_CONF_DATA_USE_PAGE_SIZE, _SC_PAGESIZE, offsetof(SysConfData, pageSize)},
#endif
#if LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_PHYSICAL_PAGES
    {LINUX_SYS_CONF_DATA_USE_PHYSICAL_PAGES, _SC_PHYS_PAGES, offsetof(SysConfData, physicalPages)},
#endif
#if LINUX_SYS_CONF_DATA_MY_USE & LINUX_SYS_CONF_DATA_USE_PHYSICAL_PAGES_AVAILABLE
    {LINUX_SYS_CONF_DATA_USE_PHYSICAL_PAGES_AVAILABLE, _SC_AVPHYS_PAGES, offsetof(SysConfData, physicalPagesAvailable)},
#endif
};

SysConfData *getSysConf(int options, SysConfData *pResult)
{
    if (pResult == 0)
    {
        pResult = (SysConfData *)calloc(1, sizeof(SysConfData));
    }

    if (pResult != 0)
    {
        for (size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++)
        {
            size_t r = 0;
            if (options & offsets[i][0])
            {
                r = sysconf(offsets[i][1]);
                char *ptr = (char *)pResult;
                size_t *tPtr = (size_t *)(ptr + offsets[i][2]);
                *tPtr = r;
            }
        }

        pResult->maxThreads = sysconf(_SC_THREAD_THREADS_MAX);
        /*
                _SC_THREAD_STACK_MIN
        */

        unsigned int support = 0;

        long int res = 0;

        for (size_t i = 0; i < sizeof(supportProps) / sizeof(supportProps[0]); i++)
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

int printSysData(SysConfData *pData, unsigned int options, char *pOutput, size_t outputLength)
{
    if (pData == 0)
    {
        return 1;
    }

#define BUF_LEN 100

    char buffer[100];
    // int outLen = outputLength > __INT32_MAX__ ? __INT32_MAX__ : (int)outputLength;
    int r = 0;
    size_t prev = 0, expected = 0;

    int l = 0;
    if (pOutput != 0)
    {
        l++;
    }

#define PREPARE() memset(buffer, 0, BUF_LEN)

#define CHECK_ADD                          \
    if (r < 0)                             \
    {                                      \
        return 1;                          \
    }                                      \
    expected += r;                         \
    if (l != 0 && expected > outputLength) \
    {                                      \
        return 1;                          \
    }                                      \
                                           \
    if (l != 0)                            \
    {                                      \
        memcpy(&pOutput[prev], buffer, r); \
    }                                      \
    else                                   \
    {                                      \
        printf("%s", buffer);              \
    }                                      \
    prev = expected;

#if LINUX_SYS_CONF_DATA_USE_NUM_CPUS
    if (options & LINUX_SYS_CONF_DATA_USE_NUM_CPUS)
    {
        PREPARE();
        r = snprintf(buffer, BUF_LEN, "CPUs: %ld\n", pData->numCpus);
        CHECK_ADD
    }
#endif

#undef CHECK_ADD
#undef PREPARE

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
// #if defined(linux)
#ifdef __unix__

    errno = 0;
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
#define LINUX_SYSCONF_BL 512

    char fullPathBuffer[LINUX_SYSCONF_BL];

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
        memset(fullPathBuffer, 0, LINUX_SYSCONF_BL);
        snprintf(fullPathBuffer, LINUX_SYSCONF_BL - 1, "/sys/devices/system/cpu/cpu%d/cache/", i);
        dr = opendir(fullPathBuffer);

        if (dr != 0)
        {
            while ((de = readdir(dr)) != NULL)
            {
                name = de->d_name;
                nameLen = strlen(name);

                if (nameLen > 5 && (0 == strncmp("index", name, 5)))
                {
                    snprintf(fullPathBuffer, LINUX_SYSCONF_BL - 1, "/sys/devices/system/cpu/cpu%d/cache/%s/", i, name);

                    DIR *indexDIR = opendir(fullPathBuffer);
                    if (indexDIR != 0)
                    {
                        printf("D: %s\n", fullPathBuffer);
                        struct dirent *indexDE;

                        int level = 1;
                        int cacheline = 0;
                        unsigned long int cpuMap = 0;
                        long int value = 0;
                        unsigned int valueUnit = 1;
                        int cacheType = -1;
                        int valid = 0;

                        while ((indexDE = readdir(indexDIR)) != NULL)
                        {
                            char *fileName = indexDE->d_name;
                            // printf("\t%s\n", fileName);

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
                                // printf("HIT: %s\n", fileName);

                                snprintf(fullPathBuffer, LINUX_SYSCONF_BL - 1, "/sys/devices/system/cpu/cpu%d/cache/%s/%s", i, name, fileName);

                                FILE *p = fopen(fullPathBuffer, "r");

                                if (p)
                                {
                                    int r;
                                    r = 1;
                                    switch (hit)
                                    {
                                    case 0:
                                        // level
                                        r = fscanf(p, "%d", &level);
                                        break;
                                    case 1:
                                        // cacheline
                                        r = fscanf(p, "%d", &cacheline);
                                        break;
                                    case 2:
                                        // shared_map
                                        r = fscanf(p, "%lx", &cpuMap);
                                        break;
                                    case 3:
                                        // size
                                        memset(valueCache, 0, sizeof(valueCache));
                                        r = fscanf(p, "%s", valueCache);
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
                                        r = fscanf(p, "%s", valueCache);
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
                                    if (r == 0)
                                    {
                                        valid |= (0x1 << hit);
                                    }

                                    fclose(p);
                                }
                            }
                        }

                        //

                        printf("L:(v=%d):%d| T:%d| LS:%d| CPUS:%lx| SZ:%ld| SZU: %d\n",
                               valid, level, cacheType, cacheline, cpuMap, value, valueUnit);
                        closedir(indexDIR);
                    }
                }
            }
            closedir(dr);
        }
    }

    printf("Assigned: %lx\n", assignedCPUs);

    return 0;
#else
#error "Unrecognized platform"
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