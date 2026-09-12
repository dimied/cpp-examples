#ifndef HTTP_PARSER_TEST
#define HTTP_PARSER_TEST

typedef struct TestRecvState
{
    size_t result;
    void *data;
    size_t size;
    size_t errorCode;
} TestRecvState;

TestRecvState testRecvState;

#define TEST_CASE_RAW_DATA(DATA) {1, DATA, sizeof(DATA), 0, 0, 0}
#define SKIP_TEST_CASE_RAW_DATA(DATA) {1, DATA, sizeof(DATA), 0, 1, 0}

#define TEST_CASE_ERRNO(ERRNO) {1, 0, -1, 0, 0, ERRNO}

typedef struct TestCase
{
    int socket;
    char *rawData;
    ssize_t result;
    int expected;
    int skipped;
    int errno_value;
} TestCase;

TestCase testCases[] = {
    TEST_CASE_RAW_DATA(""),
    TEST_CASE_ERRNO(EAGAIN),
#if EAGAIN != EWOULDBLOCK
    TEST_CASE_ERRNO(EWOULDBLOCK),
#endif
    TEST_CASE_RAW_DATA("G"),
    TEST_CASE_RAW_DATA("GET"),
    TEST_CASE_RAW_DATA("GET "),
    TEST_CASE_RAW_DATA("GET /"),
    TEST_CASE_RAW_DATA("GET / HTTP/"),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1"),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\r"),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\r\r"),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\n\r"),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\n\n"),
    TEST_CASE_RAW_DATA("GET / HTTP/1.1\r\n"),
};

#define NUM_TEST_CASES (sizeof(testCases) / sizeof(testCases[0]))

void testParser()
{
    printf("TESTING! %ld\n----\n", NUM_TEST_CASES);

    size_t testIdx;
    TestCase *pTC;
    size_t numSkipped = 0;

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

        testRecvState.data = pTC->rawData;
        testRecvState.result = pTC->result;
        test_errno = pTC->errno_value;

        MyServerClientConnection *pCC = newClientConnection();
        pCC->socketId = pTC->socket;

        int res = basicHandleClientConnection(pCC, 0);

        closeCC(pCC);
        printf("RES: %d\n", res);
        if (res != pTC->expected)
        {
            printf("FAIL:RES(exp|cur):%d|%d\n", pTC->expected, res);
            break;
        }
    }

    printf("----\n");
    printf("Completed(done/skipped/total): %ld/%ld/%ld\n", testIdx, numSkipped, NUM_TEST_CASES);
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