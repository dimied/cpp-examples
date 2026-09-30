#ifndef OBSOLETE
#define OBSOLETE


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