#ifndef FILECACHE
#define FILECACHE

typedef struct CachedFile
{
    unsigned int state;
    int dataLen;
    char *path;
    char *type;
    void *data;
    char *response;
    int responseLen;
} CachedFile;

#define CFE(PATH, TYPE) {0, 0, PATH, TYPE, 0, 0, 0}

CachedFile fileCache[3] = {
    CFE("index.html", "text/html"),
    CFE("favicon.ico", "image/x-icon"),
    CFE("console_screenshot.png", "image/png"),
};

/**
 *
 */
void *readFile(char *p, int *pSize)
{
    if (p != 0)
    {

        FILE *fp = fopen(p, "r");
        if (fp == 0)
        {
            return 0;
        }

        // calculating the size of the file
        fseek(fp, 0L, SEEK_END);
        int res = (int)ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        void *result = malloc(res);

        if (result != 0)
        {
            int r = fread(result, 1, res, fp);
            if (r != res)
            {
                free(result);
                result = 0;
            }
            else
            {
                if (pSize != 0)
                {
                    *pSize = res;
                }
            }
        }
        fclose(fp);
        return result;
    }
    return 0;
}

int cacheFiles()
{
    int loaded = 0;
    for (int i = 0; i < 3; i++)
    {
        if (0 == fileCache[i].state)
        {
            char *p = fileCache[i].path;
            int state = 3;
            if (p != 0)
            {
                int size = 0;
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

char *headerTmpl = "HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\nContent-Encoding: identity\r\nConnection: close\r\n\r\n";

void *searchPathInCache(char *path, int *pSize)
{
    void *result = 0;
    for (int i = 0; i < 3; i++)
    {
        if (1 == fileCache[i].state)
        {
            void *s = strstr(path, fileCache[i].path);
            if (s != 0)
            {
                printf("FOUND: %s | %s\n", fileCache[i].path, path);

                void *pData = fileCache[i].data;
                int dSize = fileCache[i].dataLen;
                int size = dSize + 100 + strlen(headerTmpl);
                char *pMem = (char *)malloc(size);
                if (pMem != 0)
                {
                    int w = snprintf(pMem, size, headerTmpl, fileCache[i].type, dSize);
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

void *searchInCache(char *path, int *pSize)
{

    char *s1 = strstr(path, "\r\n");
    if (s1 == 0)
    {
        return 0;
    }
    *s1 = 0;

    void *result = searchPathInCache(path, pSize);

    *s1 = '\r';
    return result;
}

char *favData = 0;
int favDataLen = 0;
char *favResponse = 0;
int favResponseLen = 0;
char *favHeader = "HTTP/1.0 200 OK\r\nContent-Type: image/x-icon\r\nContent-Length: %d\r\nContent-Encoding: identity\r\n\r\n";

int loadFAV()
{
    int result = 0;

    if (favData == 0)
    {
        FILE *fp = fopen("./favicon.ico", "r");
        if (fp != 0)
        {
            fseek(fp, 0L, SEEK_END);

            // calculating the size of the file
            long int res = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            favData = malloc(res);
            if (favData != 0)
            {
                int r = fread(favData, 1, res, fp);
                if (r != res)
                {
                    printf("Reading failed!\n");
                    free(favData);
                    favData = 0;
                    result = 1;
                }
                else
                {
                    favDataLen = res;
                }
            }
            else
            {
                result = 2;
            }
            fclose(fp);
            return result;
        }
        else
        {
            printf("favicon.ico missing!\n");
            result = 3;
        }
    }
    if (result == 0 && favResponse == 0)
    {
        int s = strlen(favHeader) + 16;
        char *resp = malloc(s + favDataLen);

        if (resp != 0)
        {
            memset(resp, 0, s + favDataLen);
            sprintf(resp, favHeader, favDataLen);
            s = strlen(resp);
            memcpy(resp + s, favData, favDataLen);
            favResponseLen = s + favDataLen;
            favResponse = resp;
        }
    }
    return result;
}
#endif