#ifndef PRINT_UTIL_INCLUDED
#define PRINT_UTIL_INCLUDED

void print_bytes(unsigned char *bytes, size_t num, size_t max, int per_line, int sep)
{
    if (num > max)
    {
        num = max;
    }
    size_t i;
    for (i = 0; i < num; i++)
    {
        if (i != 0 && i % per_line == 0)
        {
            printf("\n");
        }
        if (sep)
        {
            printf(" ");
        }
        printf("%02x", bytes[i]);
    }
    printf("\n");
}

void writeToTestResultFile(int i, int j, char *data)
{
    char buf[32];

    if (data != 0)
    {
        memset(buf, 0, 32);
        sprintf(buf, "comp%d_%d.temp.txt", i, j);
        FILE *fp = fopen(buf, "wb");
        if (fp != 0)
        {
            size_t s = strlen(data);
            size_t r = fwrite(data, 1, s, fp);
            if (r != s)
            {
                printf("ERR: No all data written!\n");
            }
            fclose(fp);
        }
    }
    else
    {
        printf("ERR:NO TEST RESULT DATA!\n");
    }
}

#endif