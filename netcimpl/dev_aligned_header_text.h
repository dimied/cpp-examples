#ifndef DEV_ALIGNED_HEADER_TEXT
#define DEV_ALIGNED_HEADER_TEXT

typedef struct DevAlignedHeadersText
{
    size_t alignedSize;
    size_t realSize;
    size_t minSize;
    size_t maxSize;
    uint64_t sizes;
    int numHits;
    char hits[NUM_HEADER_HANDLERS];
    size_t sizeCounts[64];
    uint64_t offsets[64];
    char *names[64][8];
    uint64_t values[64];
} DevAlignedHeadersText;

#endif