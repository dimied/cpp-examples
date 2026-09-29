#ifndef HTTP_PARSER_TEST
#define HTTP_PARSER_TEST

typedef struct TestRecvState
{
    ssize_t result;
    /*@null@*/ void *data;
    size_t size;
    size_t errorCode;
    size_t numChunks;
    /*@null@*/ size_t *chunks;
} TestRecvState;

TestRecvState testRecvState;

#define TEST_NOT_SKIPPED 0
#define TEST_SKIPPED 1
#define TEST_NO_ERRNO 0
#define TEST_RECV_FAILED -1
#define TEST_NO_DATA ((void *)0)
#define TEST_NO_CHUNKS ((TestCaseChunkInfo *)0)

#define TEST_CASE_NO_DATA(EXPECTED) \
    {                               \
        (int)TEST_NOT_SKIPPED,      \
        (int)1,                     \
        (int)EXPECTED,              \
        (int)0,                     \
        TEST_NO_DATA,               \
        (ssize_t)0,                 \
        TEST_NO_ERRNO,              \
        TEST_NO_CHUNKS}

#define TEST_CASE_RAW_DATA(DATA, EXPECTED) \
    {                                      \
        (int)TEST_NOT_SKIPPED,             \
        (int)1,                            \
        (int)EXPECTED,                     \
        (int)0,                            \
        DATA,                              \
        (ssize_t)(sizeof(DATA) - 1),       \
        TEST_NO_ERRNO,                     \
        TEST_NO_CHUNKS}

#define SKIP_TEST_CASE_RAW_DATA(DATA, EXPECTED) \
    {                                           \
        (int)TEST_SKIPPED,                      \
        (int)1,                                 \
        (int)EXPECTED,                          \
        (int)0,                                 \
        DATA,                                   \
        (ssize_t)(sizeof(DATA) - 1),            \
        TEST_NO_ERRNO,                          \
        TEST_NO_CHUNKS}

#define TEST_CASE_ERRNO(ERRNO, EXPECTED) \
    {                                    \
        (int)TEST_NOT_SKIPPED,           \
        (int)1,                          \
        (int)EXPECTED,                   \
        (int)0,                          \
        TEST_NO_DATA,                    \
        (ssize_t)TEST_RECV_FAILED,       \
        ERRNO,                           \
        TEST_NO_CHUNKS}

typedef struct TestCaseChunkInfo
{
    size_t numChunks;
    int *chunkSizes;
    int *results;
} TestCaseChunkInfo;

typedef struct TestCase
{
    int skipped;
    int socket;
    int expected;
    int reserved;

    /*@null@*/ char *rawData;
    ssize_t result;
    int errno_value;

    /*@null@*/ TestCaseChunkInfo *chunks;
} TestCase;

TestCase testCases[] = {
    TEST_CASE_ERRNO(EAGAIN, RECV_PARSE_ERROR_EAGAIN),
#if EAGAIN != EWOULDBLOCK
    TEST_CASE_ERRNO(EWOULDBLOCK, RECV_PARSE_ERROR_EAGAIN),
#endif
    TEST_CASE_NO_DATA(RECV_PARSE_ERROR_END_OF_FILE),
    TEST_CASE_RAW_DATA("G", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET ", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET /", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET / HTTP/", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\r", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\r\r", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\n\r", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\n\n", RECV_PARSE_ERROR_FAILED),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\r\n", RECV_PARSE_ERROR_FAILED),
};

#define NUM_TEST_CASES (sizeof(testCases) / sizeof(testCases[0]))

void testParser()
{
    printf("TESTING! #tests=%d\n----\n", (int)NUM_TEST_CASES);

    int testIdx;
    TestCase *pTC;
    int numSkipped = 0, numFails = 0;

    for (testIdx = 0; testIdx < (int)NUM_TEST_CASES; testIdx++)
    {
        printf("TEST: %d", (int)testIdx);
        pTC = &testCases[testIdx];

        if (pTC->skipped != 0)
        {
            printf(" skipped!\n");
            ++numSkipped;
            continue;
        }
        else
        {
            printf("\n");
        }

        nullify(&testRecvState, sizeof(testRecvState));

        testRecvState.result = pTC->result;
        // printf("DL: %ld\n", pTC->result);
        testRecvState.data = pTC->rawData;
#if TESTING
        test_errno = pTC->errno_value;
#endif
        MyServerClientConnection *pCC = newClientConnection();
        if (pCC == (MyServerClientConnection *)0)
        {
            break;
        }

        pCC->socketId = pTC->socket;

        int res = -1;

        if (pTC->chunks != 0)
        {
            size_t numChunks = pTC->chunks->numChunks;
            size_t cIdx;
            for (cIdx = 0; cIdx < numChunks; cIdx++)
            {
                res = basicHandleClientConnection(pCC, 0);
            }
        }
        else
        {
            res = basicHandleClientConnection(pCC, 0);
        }

        closeCC(pCC);
        printf("RES: %d\n", res);
        if (res != pTC->expected)
        {
            printf("FAIL:RES(exp|cur):%d|%d\n", pTC->expected, res);
            ++numFails;
        }
        printf("\n");
    }

    printf("----\n");
    printf("Completed(done/fails/skipped/total): %d/%d/%d/%d\n", testIdx - numFails, numFails, numSkipped, (int)NUM_TEST_CASES);
    printf("----\n");
}

TestRecvState *getTestState(int fd)
{
    (void)(fd);
    return &testRecvState;
}

ssize_t test_recv(int __fd, void *__buf, size_t __n, int __flags)
{
    (void)(__flags);
    TestRecvState *state = getTestState(__fd);

    if (state->result > 0)
    {
        size_t dataLen = (size_t)state->result;
        if (dataLen > __n)
        {
            dataLen = __n;
        }
        if (state->data == 0)
        {
            return -1;
        }
        memcpy(__buf, state->data, dataLen);
    }

    return state->result;
}

void testHeaders()
{
    printf("Test headers!\n");
    HttpRequest *pRequest = newRequest();

    HeadersDataInfo info;

    clearHeadersDataInfo(&info);

    printHeadersDataInfo(&info);

    char *test_dataValues[] = {
        "my data",
        "no data",
        "",
        "hello",
        "some new data\r\n",
        "Another data",
        "sldfkditugzetj8g847546hg4f876jz5486g45f6j457j6g476754g7f8j456764j5g6fj7456",
        "Just another test value\n",
        "Just another test value\r\n",
        "Just another test value2\r\n",
        "Just another test value2\r\n"};

    const size_t numValues = sizeof(test_dataValues) / sizeof(test_dataValues[0]);

    size_t keepRuning = 1, expected = 0;

    int run;
    for (run = 0; keepRuning != 0 && run < 1000; run++)
    {
        size_t i;
        for (i = 0; i < numValues; i++)
        {
            int l = (int)strlen(test_dataValues[i]);

            int r = addHeadersData(pRequest, test_dataValues[i], l);

            expected += l;

            if (r != ADD_HEADERS_OK)
            {
                printf("Failed: @run=%d,test=%d|%s|%d|\n", run, r, test_dataValues[i], l);
                keepRuning = 0;
                break;
            }
            r = getHeadersDataInfo(pRequest, &info);
            if (r != 0)
            {
                printf("get failed!\n");
                keepRuning = 0;
                break;
            }
            if (info.used != (unsigned int)expected)
            {
                printf("Fail: exp=%u | cur=%u\n", (unsigned int)expected, info.used);
                keepRuning = 0;
                break;
            }
            printHeadersDataInfo(&info);
        }
    }

    freeRequest(pRequest);
}

#endif