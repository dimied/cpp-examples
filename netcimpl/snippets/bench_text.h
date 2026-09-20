
#include <ctype.h>

void strtolower(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        str[i] = tolower((unsigned char)str[i]);
    }
}

char *parse_text1(char *pathStart, int headerState)
{
    char *pSep = 0;
    while (*pathStart != '\n')
    {
        char c = *pathStart;
        if (c < 0)
        {
            headerState++;
            break;
        }

        if (c == ':')
        {
            pSep = pathStart;
            break;
        }

        if (c >= 'A' && c <= 'Z')
        {
            *pathStart = (char)(c + 32);
        }

        ++pathStart;
    }
    return pSep;
}

#define AAA (67108862ul) // (((0x1ul << ('Z' - 'A')) - 1) << 1)

char *parse_text_bits(char *pathStart, int headerState)
{
    char *pSep = 0;

    while (*pathStart != '\n')
    {
        char c = *pathStart;
        if (c < 0)
        {
            headerState++;
            break;
        }

        if (c == ':')
        {
            pSep = pathStart;
            break;
        }
        uint64_t off = (c >> 6);
        c &= 0x3F;
        off &= (AAA >> c);

        if (off != 0)
        {
            *pathStart = (char)(c + 96); // 96 = 64 + 32
        }

        ++pathStart;
    }
    return pSep;
}

#if 0
int func1b(char* hostname, int host_port) {

    int c = 6;
    uint64_t v = 0;
    while (*hostname != 0 && c!=0)
    {
        char val = *hostname;
        if (val > '9')
        {
            return 1;
        }
        host_port = host_port * 10 + (*hostname - '0');
        v |= (uint64_t)0x1ul<<val;
        ++hostname;
        --c;
    }
    if(*hostname==0 && c > 0 && v>=(0x1ul<<'0')&& v<=(0x1ul<<'9')) {
        return host_port;
    }
    return 1;            
}
#endif

void bench_parse_text()
{
    char *testData = calloc(1024, 1);

    printf("BO: %lu\n", AAA);

    if (testData != 0)
    {
        char *testData2 = calloc(1024, 1);
        if (testData2 == 0)
        {
            free(testData);
            return;
        }
        char *testData3 = calloc(1024, 1);
        if (testData3 == 0)
        {
            free(testData2);
            free(testData);
            return;
        }
        testData[1022] = '\n';
        size_t i;
        for (i = 0; i < 1022; i++)
        {
            int r = rand();
            r %= 7;
            int c = rand();
            c %= ('Z' - 'A');
            if (r < 4)
            {
                c += 'A';
            }
            else
            {
                c += 'a';
            }
            testData[i] = (char)c;
        }
        printf("T:%s|\n", testData);
        memcpy(testData2, testData, 1024);
        strtolower(testData2);
        printf("L:%s|\n", testData2);

        memcpy(testData3, testData, 1024);

        clock_t start, end;

        size_t total = NUM_BENCH_TEXT_RUNS * NUM_BENCH_TEXT_ENTRIES;

        printf("CASES!#runs:%d|#tests:%d|#total:%ld\n", NUM_BENCH_TEXT_RUNS, NUM_BENCH_TEXT_ENTRIES, total);
        size_t testRun;

        printf("Normal...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_TEXT_RUNS; testRun++)
        {
            for (i = 0; i < NUM_BENCH_TEXT_ENTRIES; i++)
            {
                int headerState = 0;

                char *r = parse_text1(testData, headerState);
                if (r != 0)
                {
                    printf("FAILED: %ld|%ld\n", testRun, i);
                    break;
                }
            }
        }

        end = clock();
        printTimes("Normal", start, end, total);

        printf("Bits...\n");
        start = clock();

        for (testRun = 0; testRun < NUM_BENCH_TEXT_RUNS; testRun++)
        {
            for (i = 0; i < NUM_BENCH_TEXT_ENTRIES; i++)
            {
                int headerState = 0;

                char *r = parse_text_bits(testData3, headerState);
                if (r != 0)
                {
                    printf("FAILED: %ld|%ld\n", testRun, i);
                    break;
                }
            }
        }

        end = clock();
        printTimes("Bits", start, end, total);
        //printf("T3:%s|\n", testData3);

        free(testData3);
        free(testData2);
        free(testData);
    }
}