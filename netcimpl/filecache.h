#ifndef FILECACHE
#define FILECACHE

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

/**
 *
 */

static /*@null@*/ void *close_file_and_return(FILE *fp)
{
#ifdef S_SPLINT_S
    (void)fclose(fp);
#else
    fclose(fp);
#endif
    return (void *)0;
}

static /*@null@*/ void *readFile(char *p, size_t *pSize)
{
    if (p != 0)
    {
        FILE *fp = fopen(p, "r");
        if (((FILE *)0) == fp)
        {
            return (void *)0;
        }

        // calculating the size of the file
        {
            int r = fseek(fp, 0L, SEEK_END);
            if (r != 0)
            {
                return close_file_and_return(fp);
            }
        }
        long res = ftell(fp);
        if (res < 0)
        {
            return close_file_and_return(fp);
        }
        {
            int r = fseek(fp, 0L, SEEK_SET);
            if (r != 0)
            {
                return close_file_and_return(fp);
            }
        }

        void *result = malloc((size_t)res);

        if (result != 0)
        {
            size_t r = fread(result, 1, (size_t)res, fp);
            if (r != (size_t)res)
            {
                free(result);
                result = 0;
            }
            else
            {
                if (pSize != 0)
                {
                    *pSize = (size_t)res;
                }
            }
        }
#ifdef S_SPLINT_S
        (void)fclose(fp);
#else
        fclose(fp);
#endif
        return result;
    }
    return 0;
}

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

static /*@null@*/ char *favData = 0;
static size_t favDataLen = 0;
static /*@null@*/ char *favResponse = 0;
static size_t favResponseLen = 0;
#define FILECACHE_HTTP_FAVICON_HEADER "HTTP/1.0 200 OK\r\nContent-Type: image/x-icon\r\nContent-Length: %d\r\nContent-Encoding: identity\r\n\r\n"
static const char *favHeader = FILECACHE_HTTP_FAVICON_HEADER;

int loadFAV()
{
    int result = 0;

    if (favData == (char *)0)
    {
        FILE *fp = fopen("./favicon.ico", "r");
        if (fp != ((FILE *)0))
        {
            {
                int r = fseek(fp, 0L, SEEK_END);
                if (r != 0)
                {
                    return 100;
                }
            }

            // calculating the size of the file
            long int res = ftell(fp);
            {
                int r = fseek(fp, 0L, SEEK_SET);
                if (r != 0)
                {
                    return 100;
                }
            }

            favData = malloc((size_t)res);
            if (favData != 0)
            {
                const size_t r = fread(favData, 1, (size_t)res, fp);
                if (r != (size_t)res)
                {
                    printf("Reading failed!\n");
                    free(favData);
                    favData = 0;
                    result = 1;
                }
                else
                {
                    favDataLen = (size_t)res;
                }
            }
            else
            {
                result = 2;
            }
#ifdef S_SPLINT_S
            (void)fclose(fp);
#else
            fclose(fp);
#endif
            return result;
        }
        else
        {
            printf("favicon.ico missing!\n");
            result = 3;
        }
    }

    if (result == 0 && favResponse == 0 && favData != (char *)0)
    {
        size_t s = strlen(favHeader) + 16;
        size_t resp_size = s + favDataLen;
        char *resp = malloc(resp_size);

        if (resp != 0)
        {
            memset(resp, 0, s + favDataLen);

            int res = snprintf(resp, resp_size, FILECACHE_HTTP_FAVICON_HEADER, (int)favDataLen);
            if (res >= (int)resp_size)
            {
                free(resp);
                return 200;
            }
            s = (size_t)res;
            memcpy(resp + s, favData, favDataLen);
            favResponseLen = s + favDataLen;
            favResponse = resp;
        }
    }
    return result;
}
#endif