#ifndef FILE_UTIL_INCLUDED
#define FILE_UTIL_INCLUDED

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

static /*@null@*/ void *readFile(const char *p, size_t *pSize)
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

#endif