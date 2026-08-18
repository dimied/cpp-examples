
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

// defined in sys/resource.h
//*
// struct rlimit {
//     rlim_t cur;    //current limit
//     rlim_t max;    //ceiling for cur.
// }
//*//

// Bytes To GigaBytes
static inline unsigned long btogb(unsigned long bytes)
{
    return bytes / (1024 * 1024 * 1024);
}

// Bytes To ExaBytes
// static inline double btoeb(double bytes)
//{
//    return bytes / (1024.00 * 1024.00 * 1024.00 * 1024.00 * 1024.00 * 1024.00);
//}

int main()
{

    struct rlimit rlim;
    rlim_t addr_space;

    /*
     * Here we call to getrlimit(), with RLIMIT_AS (Address Space) and
     * a pointer to our instance of rlimit struct.
     */
    int retval = getrlimit(RLIMIT_CPU, &rlim);
    if (retval == 0)
    {
        printf("CPU:        RLIMIT_CPU: soft: %lu secs\n", rlim.rlim_cur);
    }

    retval = getrlimit(RLIMIT_CORE, &rlim);
    if (retval == 0)
    {
        printf("Core file: RLIMIT_CORE: soft: %lu bytes\n", rlim.rlim_cur);
    }

    retval = getrlimit(RLIMIT_AS, &rlim);
    if (retval == 0)
    {
        addr_space = rlim.rlim_cur;
        fprintf(stdout, "Address_space: RLIMIT_AS: %lu Bytes, or %lu GB\n", addr_space, btogb(addr_space));
    }
    else
    {
        fprintf(stderr, "Address_space: RLIMIT_AS: Coundn\'t get address space current limit.\n");
    }

    rlim.rlim_cur = (4UL) * (1024) * 1024 * 1024;
    retval = setrlimit(RLIMIT_AS, &rlim);
    if (retval == 0)
    {
        printf("ASSIGNED: RLIMIT_AS\n");
    }
    retval = getrlimit(RLIMIT_AS, &rlim);
    if (retval == 0)
    {
        addr_space = rlim.rlim_cur;
        fprintf(stdout, "Address_space: RLIMIT_AS: %lu Bytes, or %lu GB\n", addr_space, btogb(addr_space));
    }

    retval = getrlimit(RLIMIT_DATA, &rlim);
    if (retval == 0)
    {
        printf("Data segment: RLIMIT_DATA: soft: %lu bytes\n", rlim.rlim_cur);
    }
    rlim.rlim_cur = (4UL) * (1024) * 1024 * 1024;
    retval = setrlimit(RLIMIT_DATA, &rlim);
    if (retval == 0)
    {
        printf("ASSIGNED: RLIMIT_DATA\n");
    }
    retval = getrlimit(RLIMIT_DATA, &rlim);
    if (retval == 0)
    {
        printf("Data segment: RLIMIT_DATA: soft: %lu bytes\n", rlim.rlim_cur);
    }

    retval = getrlimit(RLIMIT_FSIZE, &rlim);
    // SIGFSZ signal
    if (retval == 0)
    {
        printf("Max file size: RLIMIT_FSIZE: soft: %lu bytes\n", rlim.rlim_cur);
    }

    retval = getrlimit(RLIMIT_LOCKS, &rlim);
    if (retval == 0)
    {
        printf("Locks: RLIMIT_LOCKS: soft: %lu locks\n", rlim.rlim_cur);
    }

    retval = getrlimit(RLIMIT_MEMLOCK, &rlim);
    if (retval == 0)
    {
        printf("Mem locks: RLIMIT_MEMLOCK: soft: %lu locks\n", rlim.rlim_cur);
    }

    retval = getrlimit(RLIMIT_MSGQUEUE, &rlim);
    if (retval == 0)
    {
        printf("Msg queue: RLIMIT_MSGQUEUE: soft: %lu bytes\n", rlim.rlim_cur);
    }

    retval = getrlimit(RLIMIT_NPROC, &rlim);
    if (retval == 0)
    {
        printf("Threads: RLIMIT_NPROC: soft: %lu : max: %lu\n", rlim.rlim_cur, rlim.rlim_max);
    }

    retval = getrlimit(RLIMIT_STACK, &rlim);
    if (retval == 0)
    {
        printf("Stack: RLIMIT_STACK: soft: %lu bytes\n", rlim.rlim_cur);
    }

    retval = getrlimit(RLIMIT_SIGPENDING, &rlim);
    if (retval == 0)
    {
        printf("Signals: RLIMIT_SIGPENDING: soft: %lu\n", rlim.rlim_cur);
    }
    // Missing:...
    // RLIMIT_NICE
    // RLIMIT_NOFILE

    return 0;
}
