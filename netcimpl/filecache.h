#ifndef FILECACHE
#define FILECACHE

#include "file_util.h"

typedef struct CachedFile
{
    int state;
    size_t dataLen;
    char *path;
    char *type;
    /*@null@*/ void *data;
    /*@null@*/ char *response;
    int responseLen;
} CachedFile;

#define CFE(PATH, TYPE) {0, 0, PATH, TYPE, (void *)0, (char *)0, 0}

static CachedFile fileCache[3] = {
    CFE("index.html", "text/html"),
    CFE("favicon.ico", "image/x-icon"),
    CFE("console_screenshot.png", "image/png"),
};

#define FILECACHE_HEADER_TEMPLATE "HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent-Length: %lu\r\nContent-Encoding: identity\r\nConnection: close\r\n\r\n"


static int cacheFiles()
{
    int loaded = 0;
    int i;
    for (i = 0; i < 3; i++)
    {
        if (0 == fileCache[i].state)
        {
            char *p = fileCache[i].path;
            int state = 3;
            if (p != 0)
            {
                size_t size = 0;
                void *data = readFile(p, &size);
                printf("Load: %s", p);

                if (data != 0)
                {
                    state = 1;
                    fileCache[i].data = data;
                    fileCache[i].dataLen = size;
                    printf(" loaded!\n");
                    loaded++;
                }
                else
                {
                    state = 2;
                    printf(" NOT loaded!\n");
                }
            }
            fileCache[i].state = state;
        }
    }
    return loaded;
}

static /*@null@*/ void *searchPathInCache(char *path, size_t *pSize)
{
    void *result = 0;
    int i;
    for (i = 0; i < 3; i++)
    {
        if (1 == fileCache[i].state)
        {
            void *s = strstr(path, fileCache[i].path);
            if (s != 0)
            {
                printf("FOUND: %s | %s\n", fileCache[i].path, path);

                void *pData = fileCache[i].data;
                if (pData == 0)
                {
                    continue;
                }

                size_t dSize = fileCache[i].dataLen;
                size_t size = dSize + 100 + strlen(FILECACHE_HEADER_TEMPLATE);

                char *pMem = (char *)malloc(size);
                if (pMem != 0)
                {
                    int w = snprintf(pMem, size, FILECACHE_HEADER_TEMPLATE, fileCache[i].type, (unsigned long)dSize);
                    char *p = pMem + w;
                    memcpy(p, pData, dSize);
                    dSize += w;
                    if (pSize != 0)
                    {
                        *pSize = dSize;
                    }
                    result = pMem;
                }

                break;
            }
        }
    }
    return result;
}

#if 0
static /*@null@*/ void *searchInCache(char *path, size_t *pSize)
{

    char *s1 = strstr(path, "\r\n");

    if (s1 == (char *)0)
    {
        return 0;
    }
    *s1 = '\0';

    void *result = searchPathInCache(path, pSize);

    *s1 = '\r';
    return result;
}
#endif

#endif