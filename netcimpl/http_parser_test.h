#ifndef HTTP_PARSER_TEST
#define HTTP_PARSER_TEST

typedef struct TestRecvState
{
    size_t result;
    void *data;
    size_t size;
    size_t errorCode;
    size_t numChunks;
    size_t *chunks;
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
        TEST_NOT_SKIPPED,           \
        1,                          \
        EXPECTED,                   \
        0,                          \
        TEST_NO_DATA,               \
        0,                          \
        TEST_NO_ERRNO,              \
        TEST_NO_CHUNKS}

#define TEST_CASE_RAW_DATA(DATA, EXPECTED) \
    {                                      \
        TEST_NOT_SKIPPED,                  \
        1,                                 \
        EXPECTED,                          \
        0,                                 \
        DATA,                              \
        sizeof(DATA) - 1,                  \
        TEST_NO_ERRNO,                     \
        TEST_NO_CHUNKS}

#define SKIP_TEST_CASE_RAW_DATA(DATA, EXPECTED) \
    {                                           \
        TEST_SKIPPED,                           \
        1,                                      \
        EXPECTED,                               \
        0,                                      \
        DATA,                                   \
        sizeof(DATA) - 1,                       \
        TEST_NO_ERRNO,                          \
        TEST_NO_CHUNKS}

#define TEST_CASE_ERRNO(ERRNO, EXPECTED) \
    {                                    \
        TEST_NOT_SKIPPED,                \
        1,                               \
        EXPECTED,                        \
        0,                               \
        TEST_NO_DATA,                    \
        TEST_RECV_FAILED,                \
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

    char *rawData;
    ssize_t result;
    int errno_value;

    TestCaseChunkInfo *chunks;
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
    printf("TESTING! %ld\n----\n", NUM_TEST_CASES);

    size_t testIdx;
    TestCase *pTC;
    size_t numSkipped = 0, numFails = 0;

    for (testIdx = 0; testIdx < NUM_TEST_CASES; testIdx++)
    {
        printf("TEST: %ld", testIdx);
        pTC = &testCases[testIdx];

        if (pTC->skipped)
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
        //printf("DL: %ld\n", pTC->result);
        testRecvState.data = pTC->rawData;
#if TESTING
        test_errno = pTC->errno_value;
#endif
        MyServerClientConnection *pCC = newClientConnection();
        pCC->socketId = pTC->socket;

        int res;

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
    printf("Completed(done/fails/skipped/total): %ld/%ld/%ld/%ld\n", testIdx - numFails, numFails, numSkipped, NUM_TEST_CASES);
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

#endif