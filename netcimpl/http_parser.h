#ifndef HTTP_PARSER
#define HTTP_PARSER

// #include "mimetypecreate.h"

#define SERVER_MULTITHREADING 0

#if SERVER_MULTITHREADING
#include <pthread.h>

#define THREAD_LOCAL __thread
#else
#define THREAD_LOCAL
#endif

#include "http_parser_header_constants.h"
#include "http_parser_macros.h"
#include "http_parser_types.h"
#include "http_parser_header_functions.h"

#ifdef HTTP_PARSER_DECLARATIONS
#if HTTP_PARSER_DECLARATIONS

#define ADD_HEADERS_OK 0
#define ADD_HEADERS_INVALID_ARGS 1
#define ADD_HEADERS_NOMEM 2
#define ADD_HEADERS_TOO_LARGE 3

#define HEADER_CHUNK_SIZE 128

size_t header_size(int len)
{
    unsigned int ds = (len + 7) & 0xFFFFFF0;
    if (ds < HEADER_CHUNK_SIZE)
    {
        return HEADER_CHUNK_SIZE;
    }
    else
    {
        return (ds + 63) & 0xFFFFFFC0; // 64 byte aligned
    }
}

// TODO:instead of saving full header name,
//  we can save (0x80 | HEADER_IDX) as first byte and reducing the amount of bytes
// for unknown headers we save full header name.
int addHeadersData(HttpRequest *pReq, char *data, int len)
{
    if (pReq != 0 && data != 0 && len >= 0 && len < (0x1 << 20))
    {
        if (len == 0)
        {
            return ADD_HEADERS_OK;
        }

        void *headers = pReq->__headers;

        if (headers == 0)
        {
            size_t s = header_size(len + sizeof(HeadersDataHeader));
            // printf("SZ:%d>%ld\n", len, s);

            if (s >= 0x8000)
            {
                // TODO
                return ADD_HEADERS_TOO_LARGE;
            }

            headers = calloc(s, 1);

            if (headers != 0)
            {
                HeadersDataHeader *pHeader = (HeadersDataHeader *)headers;
                char *dstData = (char *)headers + sizeof(HeadersDataHeader);
                memcpy(dstData, data, len);
                s -= sizeof(HeadersDataHeader);
                pHeader->size_and_used = (s << 16) | len;
                pReq->__headers = headers;
                return ADD_HEADERS_OK;
            }
            return ADD_HEADERS_NOMEM;
        }
        else
        {
            HeadersDataHeader *pHeader = (HeadersDataHeader *)headers;

            if (pHeader->next == 0)
            {
                size_t newSize = pHeader->size_and_used >> 16;
                size_t used = pHeader->size_and_used & 0xFFFF;
                // can insert?
                if (len + used < newSize)
                {
                    // printf("HEADER:FIT!\n");
                    char *dstData = (char *)headers + sizeof(HeadersDataHeader);
                    dstData += used;
                    // printf("COPY: [%p,%p) -> [%p,%p]\n", dstData, dstData + len, headers, (char *)headers + sizeof(HeadersDataHeader) + newSize);
                    memcpy(dstData, data, len);
                    used += len;
                    pHeader->size_and_used = (pHeader->size_and_used & 0xFFFF0000) | used;
                    return ADD_HEADERS_OK;
                }
                newSize += header_size(len) + sizeof(HeadersDataHeader);
                newSize = header_size(newSize);

                if (newSize >= 0x8000)
                {
                    // TODO
                    return ADD_HEADERS_TOO_LARGE;
                }
                // printf("HEADER:REALLOC! %ld|%d > %ld\n", pHeader->size_and_used >> 16, len, newSize);
                // printf("REALLOC: %p", headers);

                // try to reallocate
                char *res = (char *)realloc(headers, newSize);
                // printf(" | %p\n", res);
                // printf("RANGE:[%p , %p)\n", res, res + newSize);

                if (res != 0)
                {
                    pHeader = (HeadersDataHeader *)res;
                    const size_t dontTouchSize = sizeof(HeadersDataHeader) + used;
                    char *dstData = (char *)res + dontTouchSize;
                    // clear new memory
                    memset(dstData, 0, newSize - dontTouchSize);
                    memcpy(dstData, data, len);

                    used = dontTouchSize - sizeof(HeadersDataHeader) + len;
                    newSize -= sizeof(HeadersDataHeader);
                    pHeader->size_and_used = (newSize << 16) | used;
                    pReq->__headers = res;
                    return 0;
                }
                // TODO: add node
            }
            return 1;
        }

        return 0;
    }

    return ADD_HEADERS_INVALID_ARGS;
}

void freeHeadersData(HttpRequest *pReq)
{
    if (pReq != 0)
    {
        void *headers = pReq->__headers;
        if (headers != 0)
        {
            HeadersDataHeader *pH = (HeadersDataHeader *)headers;
            if (pH->next != 0)
            {
                perror("Header->next not implemented!\n");
                exit(1);
            }

            while (headers != 0)
            {
                pH = (HeadersDataHeader *)headers;
                void *next = pH->next;
                free(headers);
                headers = next;
            }

            free(headers);
            pReq->__headers = 0;
        }
    }
}

char *memoryForPath(int size)
{
    if (size > 0 && size < (0x1 << 20))
    {
        ++size; // if the user forgot place for '\0'
        size = (size + 7) & 0xFFFFFFF8;
        return (char *)calloc(size >> 3, 8);
    }
    return 0;
}

void freeMemoryForPath(void *pMem)
{
    if (pMem != 0)
    {
        free(pMem);
    }
}

HttpRequest *newRequest()
{
    // size_t s = (sizeof(HttpRequest) + 31) & 0xFD0;
    size_t s = sizeof(HttpRequest);
    printf("RA: %ld\n", s);
    return (HttpRequest *)calloc((s + 7) >> 3, 8);
}

void freeRequest(void *p)
{
    if (p != 0)
    {
        HttpRequest *pReq = (HttpRequest *)p;
        if (pReq->path != 0)
        {
            freeMemoryForPath(pReq->path);
            pReq->path = 0;
        }
        freeHeadersData(pReq);
        free(p);
    }
}

typedef struct UnknownHeaderResult
{
    size_t capacity;
    void **values;
    size_t size;
} UnknownHeaderResult;

typedef int (*HeaderHandlerFunc)(MyServerClientConnection *pCC, char *data, int len);

typedef struct HeaderHandler
{
    char *name;
    HeaderHandlerFunc func;
} HeaderHandler;

typedef struct AcceptType
{
    unsigned int size;
    unsigned int type;
    const char *str;
} AcceptType;

#endif
#endif

#ifndef HTTP_PARSER_LOOKUP
#define HTTP_PARSER_LOOKUP

#define ENTRY_AcceptType(T, S) {(unsigned int)sizeof(S), T, S}
// to speed up decide by first char (range: '*' - 'z')
AcceptType acceptTypes[] = {
    ENTRY_AcceptType(MIME_TYPE_ANY, "*/*"),
    ENTRY_AcceptType(MIME_TYPE_XHTML_XML, "application/xhtml+xml"),
    ENTRY_AcceptType(MIME_TYPE_XML, "application/xml"),
    ENTRY_AcceptType(MIME_TYPE_HTML, "text/html"),
    ENTRY_AcceptType(MIME_TYPE_IMAGE_ANY, "image/*"),
};

#define NUM_ACCEPT_TYPES (sizeof(acceptTypes) / sizeof(acceptTypes[0]))

#endif

#include "http_parser_headers.h"

#ifndef BUF_LEN_IN
#define BUF_LEN_IN 4096
#endif

#if BUF_LEN_IN < 64
#error "Buffer length too small."
#endif

#ifdef BUF_LEN_IN_64
#error "BUF_LEN_IN_64 defined"
#else
#define BUF_LEN_IN_64 ((BUF_LEN_IN + 7) >> 3)
#endif

#ifdef SET_STATES
#define INC_STATE_OR_SET(STATE, NEW_STATE) (STATE = NEW_STATE)
#else
#define INC_STATE_OR_SET(STATE, NEW_STATE) (STATE++)
#endif

#define PARSE_STATE_REQ_LINE 0
#define PARSE_STATE_REQ_LINE_HAS_METHOD 1
#define PARSE_STATE_REQ_LINE_PATH 2
#define PARSE_STATE_REQ_LINE_PROTOCOL 3
#define PARSE_STATE_REQ_HEADERS 4

char *parseStates[] = {
    "Line",
    "Method",
};
char *noneString = "(nil)";

void printRequest(HttpRequest *p)
{
    if (p != 0)
    {
        printf(">Req:\n");

        int m = p->requestInfo & 0x1F;
        const char *s;
        if (m < 9)
        {
            s = http_methods[m];
        }
        else
        {
            s = noneString;
        }
        printf(">METHOD|%s|\n", s);
        s = noneString;
        if (p->path)
        {
            s = p->path;
        }
        printf(">PATH|%s|\n", s);
        int major = PROTOCOL_MAJOR_VERSION(p->requestInfo), minor = PROTOCOL_MINOR_VERSION(p->requestInfo);
        printf(">PROTO:%d.%d\n", major, minor);
        printf("<-\n");
    }
    else
    {
        printf("Req: (nil)\n");
    }
}

#if TESTING
int test_errno = 0;

ssize_t test_recv(int __fd, void *__buf, size_t __n, int __flags);

#define RECV_FUNC(SOCKET_ID, DATA_PTR, LEN, FLAGS) test_recv(SOCKET_ID, DATA_PTR, LEN, FLAGS)

#define ERRNO_VALUE (test_errno)

#else

#define RECV_FUNC(SOCKET_ID, DATA_PTR, LEN, FLAGS) recv(SOCKET_ID, DATA_PTR, LEN, FLAGS)

#define ERRNO_VALUE (errno)
#endif

#define RECV_PARSE_NO_ERROR 0x0

#define RECV_PARSE_ERROR_END_OF_FILE 1
#define RECV_PARSE_ERROR_NO_SOCKET 2
#define RECV_PARSE_ERROR_FAILED 3
#define RECV_PARSE_ERROR_EAGAIN 4
#define RECV_PARSE_ERRNO_BIG 8

#define RECV_PARSE_ERROR_NO_REQMEM 0x10

#define RECV_PARSE_ERROR_MORE_DATA 0x20
#define RECV_PARSE_ERROR_METHOD_FAILED 0x21

// for storing errno in the result
#define RECV_PARSE_RECV_ERROR_MASK 0x80

#define ONLY_STANDARD_HTTP_VERBS 1

THREAD_LOCAL union
{
    uint64_t data64[BUF_LEN_IN_64];
    char data[BUF_LEN_IN];
} inBuffer;

int processRequestLine(HttpRequest *pRequest)
{
    (void)(pRequest);

    return 0;
}

#define HTTP_PARSER_DEV_LOG 1
#include "http_parser_dev_log.h"

/*!< HEADER_FIELD_NAME_VALID_CHARS1_BITS:bits 0-64*/
#define HEADER_FIELD_NAME_VALID_CHARS1_BITS (0x7ff6cfa00000000ul)
/*!< HEADER_FIELD_NAME_VALID_CHARS2_BITS:bits 65-128*/
#define HEADER_FIELD_NAME_VALID_CHARS2_BITS (0x57ffffffc7fffffeul)
/*!< HEADER_FIELD_NAME_LOWERCASE_OFFSET_BITS:for uppercase*/
#define HEADER_FIELD_NAME_LOWERCASE_OFFSET_BITS (0xffffffffffffffc0ul)
// #define HEADER_FIELD_NAME_LOWERCASE_OFFSET_BITS (0xffffffffffffff80ul)

#define HOST_CHECK ('h' | ('o' << 8) | ('s' << 16) | ('t' << 24))

#ifndef SKIP_SECOND_HOST_ENTRY_CHECK
#define SKIP_SECOND_HOST_ENTRY_CHECK 0
#endif

int basicHandleClientConnection(void *pClientConnection, void *pServerConnection)
{
    (void)(pServerConnection);
    MyServerClientConnection *pCC = (MyServerClientConnection *)pClientConnection;

    const int clientSocketId = pCC->socketId;

    if (0 == clientSocketId)
    {
        return RECV_PARSE_ERROR_NO_SOCKET;
    }

    int state = GET_PARSE_STATE(pCC);

    int readChunks = 1;

    // reading in chunks
    while (readChunks != 0)
    {
        --readChunks;
        ssize_t recvRes;

        if (IS_SECURE_CONNECTION(pCC))
        {
            //<=0 means error
            recvRes = SSL_read(pCC->secure.ssl, inBuffer.data, BUF_LEN_IN);
        }
        else
        {
            recvRes = RECV_FUNC(clientSocketId, inBuffer.data, BUF_LEN_IN, 0);
        }
        printInfo(RECV_CODE, 0, recvRes);

        if (recvRes <= 0)
        {
            if (recvRes == 0)
            {
                return RECV_PARSE_ERROR_END_OF_FILE;
            }
            const int err = ERRNO_VALUE;

            // e.g: EAGAIN,EBADF,ECONNREFUSED,EFAULT,EINTR,EINVAL,ENOMEM,ENOTCONN,ENOTSOCK
            if (err >= 0 && err <= 0xFF)
            {
                SET_ERRNO(pCC, err);
                // TODO: correct return value
                return RECV_PARSE_ERROR_EAGAIN;
            }
            // should 'actually' never happen, errno value use only 1 byte
            return RECV_PARSE_ERRNO_BIG;
        }

        HttpRequest *pReq = pCC->request;

        if (PARSE_STATE_REQ_LINE != state && pReq == 0)
        {
            // lost request 'object'?
            return RECV_PARSE_ERROR_NO_REQMEM;
        }

        char *pathStart = inBuffer.data;
        int i = 1;
        ++recvRes;

        if (PARSE_STATE_REQ_LINE == state)
        {
            if (pReq == 0)
            {
                pReq = newRequest();

                if (pReq == 0)
                {
                    return RECV_PARSE_ERROR_NO_REQMEM;
                }
                pCC->request = pReq;
            }

            int s = 0;

#if ONLY_STANDARD_HTTP_VERBS
            // parse method
            while (i < 9 && i < recvRes)
            {
                // invalid chars or '\r', '\n'
                if (*pathStart < ' ')
                {
                    --s;
                    break;
                }
                if (*pathStart == ' ')
                {
                    ++s;
                    break;
                }
                ++pathStart;
                ++i;
            }

            // line-end found?
            if (s <= 0)
            {
                // s==0: too long
                closeBadRequest(pCC);
                return RECV_PARSE_ERROR_METHOD_FAILED;
            }
            else
            {
                *pathStart = 0;
            }
#else
#error "No custom HTTP verbs supported"
#endif
            int j;
            {
                //--i;
                for (j = 0; j < NUM_HTTP_METHODS; j++)
                {
                    if (0 == strcmp(http_methods[j], inBuffer.data))
                    {
                        break;
                    }
                }
                i += 2;
            }

            if (j < NUM_HTTP_METHODS)
            {
                printInfo(METHOD_CODE, inBuffer.data, j);
                pReq->requestInfo = j;
            }
            else
            {
                printInfo(METHOD_INVALID_CODE, inBuffer.data, j);
                closeBadRequest(pCC);
                return RECV_PARSE_ERROR_METHOD_FAILED;
            }
            ++pathStart;
            //++i;
            INC_STATE_OR_SET(state, PARSE_STATE_REQ_LINE_HAS_METHOD);
            SET_PARSE_STATE(pCC, state);
        }

        char *pStart = pathStart;
        // parse path
        if (PARSE_STATE_REQ_LINE_HAS_METHOD == state)
        {
            int s = 0;
            while (i < recvRes)
            {
                if (*pathStart < ' ')
                {
                    --s;
                    break;
                }
                if (*pathStart == ' ')
                {
                    ++s;
                    break;
                }
                ++pathStart;
                ++i;
            }
            long int diff = pathStart - pStart;
            // line-end found?
            if (s <= 0 || diff <= 0)
            {
                closeBadRequest(pCC);
                return 0;
            }
            else
            {
                // TODO: long paths
                *pathStart = 0;
            }
            ++pathStart;
            printInfo(PATH_CODE, pStart, diff);

            char *pReqPath = memoryForPath(diff);
            if (pReqPath == 0)
            {
                closeBadRequest(pCC);
                return 0;
            }
            memcpy(pReqPath, pStart, diff);
            pReq->path = pReqPath;
            INC_STATE_OR_SET(state, PARSE_STATE_REQ_LINE_PATH);
        }

        // min-size: 8 bytes
        if (PARSE_STATE_REQ_LINE_PATH == state)
        {
            int pOK = 0;
            char *protStart = pathStart;

            while (pOK < 10 && i < recvRes)
            {
                if (*pathStart <= '\n')
                {
                    if (i > 1)
                    {
                        --pathStart;
                        if (*pathStart != '\r')
                        {
                            i = -i;
                        }
                    }
                    else
                    {
                        i = -i;
                    }

                    break;
                }
                ++pOK;
                ++pathStart;
                ++i;
            }
            if (i < 0 || pOK < 6 || pOK >= 10)
            {
                closeCC(pCC);
                return 0;
            }
            else
            {
                int major, minor;
                *pathStart = 0;
                int numItems = sscanf(protStart, "HTTP/%d.%d", &major, &minor);
                *pathStart = '\r';
                if (numItems != 2 || major < 0 || major > 2 || minor < 0 || minor > 1)
                {
                    closeCC(pCC);
                    return 0;
                }
                minor |= major << 4;

                pReq->requestInfo |= ((unsigned int)minor << 8);

                pathStart += 2;
                ++i;
            }

            INC_STATE_OR_SET(state, PARSE_STATE_REQ_LINE_PROTOCOL);
            SET_PARSE_STATE(pCC, state);

            int res = processRequestLine(pReq);

            if (res != 0)
            {
                closeCC(pCC);
                return 1;
            }
        }
        printRequest(pReq);

        // parsing headers
        if (state == PARSE_STATE_REQ_LINE_PROTOCOL)
        {
            printInfo(PARSE_HEADERS_CODE, 0, NUM_HEADER_HANDLERS);
            union
            {
                uint64_t ui64[8];
                char c8[64];
            } header_name_buffer;
            memset(header_name_buffer.c8, 0, 64);

#define HO_LEN 32
            // uint32_t headerOffsets[HO_LEN];
            size_t pIdx = 0;
            int hasHost = 0;

#define HEADER_STATE_FINE 0
#define HEADER_STATE_INVALID_CHAR 1

            int headerState = HEADER_STATE_FINE;
            // TODO: Check i and recvRes
            int hIdx = 0;
            char *pHeaderStoreC8 = header_name_buffer.c8;

            const uint64_t keep_headers = REQ_HEADER_KEEP_ACCEPT | REQ_HEADER_KEEP_CONNECTION;

            while (headerState == 0 && recvRes - i >= 2 && *pathStart != '\r' && pathStart[1] != '\n')
            {
                {
                    int64_t j = pHeaderStoreC8 - &header_name_buffer.c8[0];
                    // We clear using 8 bytes, should be faster this way.
                    if (j > 0)
                    {
                        if (j > 8)
                        {
                            j >>= 3;
                            uint64_t *pHeaderStore64 = &header_name_buffer.ui64[j];

                            while (pHeaderStore64 > &header_name_buffer.ui64[0])
                            {
                                *pHeaderStore64 = 0;
                                --pHeaderStore64;
                            }
                            *pHeaderStore64 = 0;
                        }
                        else
                        {
                            header_name_buffer.ui64[0] = 0;
                        }

                        pHeaderStoreC8 = header_name_buffer.c8;
                    }
                }

                const char *pHeader = pathStart;
                char *pSep = (char *)0;
                printf("READ:HEADER:%d: (%d < %ld)\n", hIdx, i, recvRes);
                hIdx++;

                uint64_t v;
                int hl = 0;

                while (*pathStart != 0 && i < recvRes && hl < 64)
                {
                    char c = *pathStart;
                    if (c < 33)
                    {
                        headerState = HEADER_STATE_INVALID_CHAR;
                        break;
                    }

                    register int shift = (c & 0x3F);
                    v = (0x1ul) << shift;
                    char off;

                    if (c > 64)
                    {
                        v &= HEADER_FIELD_NAME_VALID_CHARS2_BITS;
                        off = (HEADER_FIELD_NAME_LOWERCASE_OFFSET_BITS >> shift) & 0x20;
                    }
                    else
                    {
                        // no lower case for values 0-64
                        v &= HEADER_FIELD_NAME_VALID_CHARS1_BITS;
                        off = 0;
                    }

                    if (v)
                    {
                        if (c == ':')
                        {
                            pSep = pathStart;
                            break;
                        }
                        if (off)
                        {
                            *pHeaderStoreC8 = c | off;
                        }
                        else
                        {
                            *pHeaderStoreC8 = c;
                        }
                    }
                    else
                    {
                        headerState = HEADER_STATE_INVALID_CHAR;
                        break;
                    }

                    ++pathStart;
                    ++pHeaderStoreC8;
                    ++i;
                    ++hl;
                }

                if (i == recvRes)
                {
                    break;
                }

                if (headerState != 0)
                {
                    // char c = *pathStart;
                    while (recvRes - i >= 2 && *pathStart != '\r' && pathStart[1] != '\n')
                    {
                        ++pathStart;
                        i++;
                    }
                    if (i < recvRes)
                    {
                        i += 2;
                        pathStart += 2;
                        headerState = 0;
                        continue;
                    }
                    // otherwise buffer exhausted
                    break;
                }
                // Now: pHeader ... pSep: header name

                if (pSep != 0)
                {
                    while (i < recvRes && *pathStart != '\n')
                    {
                        ++pathStart;
                        ++i;
                    }
                    --pathStart;

                    if (*pathStart == '\r')
                    {
                        *pathStart = 0;
                        *pSep = 0;

                        printf("HDR|%s|':'|%s|\n", header_name_buffer.c8, pSep + 1);

                        int parseNotHost = 1;

                        if (hasHost == 0)
                        {
                            if (HOST_CHECK == header_name_buffer.ui64[0])
                            {
                                int hostRes = isValidHost(pCC, pSep + 1);
                                printf("HOST!%s|%d\n", pSep + 1, hostRes);
                                if (IS_VALID_HOST_OK != hostRes)
                                {
                                    // 400 Bad Request
                                    headerState = 3;
                                    break;
                                }
                                hasHost++;
                                parseNotHost = 0;
                            }
                        }
                        if (parseNotHost)
                        {
                            int header_idx = NUM_HEADER_HANDLERS;

                            ssize_t s = pHeaderStoreC8 - header_name_buffer.c8;

                            // size within range
                            if (s >= HEADER_NAME_MIN_LEN && s <= HEADER_NAME_MAX_LEN)
                            {
                                const uint64_t bit = (0x1ul) << (s - 1);
                                // Has size?
                                if ((bit & HEADER_SIZE_BITS_CONSTANT) != 0)
                                {

                                    uint64_t offsets = header_comp_data.header_names_offsets8[s];
                                    uint64_t values = header_comp_data.header_names_values8[s];

                                    if (s > 8)
                                    {
                                        const int s2 = (s + 7) >> 3;

                                        uint64_t *vals = (uint64_t *)&header_comp_data.header_names_aligned8[0];
                                        uint64_t *pVals = header_name_buffer.ui64;
                                        int sawPrefix = 0;
                                        const uint64_t myValue = *pVals;

                                        // using loop unrolling
                                        while (values != 0)
                                        {
                                            uint64_t *checkValues = vals + (offsets & 0xFF);
                                            // checkValue = *checkValues;

                                            // 8 bytes equal?
                                            if (myValue == *checkValues)
                                            // if (*pVals == *checkValues)
                                            {
                                                sawPrefix |= 1;

                                                uint64_t *pVals2 = pVals + 1;
                                                ++checkValues;
                                                // 16 bytes equal?
                                                if (*pVals2 == *checkValues)
                                                {
                                                    sawPrefix |= 2;

                                                    if (s2 == 2)
                                                    {
                                                        header_idx = (values & 0xFF);
                                                        offsets = 0;
                                                        values = 0;
                                                        break;
                                                    }
#if HEADER_NAME_MAX_LEN > 16
                                                    else
                                                    {
                                                        ++pVals2;
                                                        ++checkValues;
                                                        // 24 bytes equal?
                                                        if (*pVals2 == *checkValues)
                                                        {
                                                            sawPrefix |= 4;
                                                            if (s2 == 3)
                                                            {
                                                                header_idx = (values & 0xFF);
                                                                offsets = 0;
                                                                values = 0;
                                                                break;
                                                            }
#if HEADER_NAME_MAX_LEN > 24
                                                            else
                                                            {
                                                                ++pVals2;
                                                                ++checkValues;
                                                                // 32 bytes equal?
                                                                if (*pVals2 == *checkValues)
                                                                {
                                                                    if (s2 == 4)
                                                                    {
                                                                        header_idx = (values & 0xFF);
                                                                        values = 0;
                                                                        offsets = 0;
                                                                        break;
                                                                    }
#if HEADER_NAME_MAX_LEN > 32
                                                                    else
                                                                    {
                                                                        ++pVals2;
                                                                        ++checkValues;
                                                                        // 33-40 bytes equal?
                                                                        if (*pVals2 == *checkValues)
                                                                        {
                                                                            header_idx = (values & 0xFF);
                                                                            values = 0;
                                                                            offsets = 0;
                                                                            break;
                                                                        }
                                                                    }
#endif
                                                                }
                                                            }
#endif
                                                        }
                                                        else if ((sawPrefix & 0x4) != 0)
                                                        {
                                                            break;
                                                        }
                                                    }
#endif
                                                }
                                                else if ((sawPrefix & 0x2) != 0)
                                                {
                                                    break;
                                                }
                                            }
                                            else if (sawPrefix != 0)
                                            {
                                                break;
                                            }

                                            offsets >>= 8;
                                            values >>= 8;
                                        }
                                    }
                                    else
                                    {
                                        const uint64_t val = header_name_buffer.ui64[0];
                                        uint64_t *vals = (uint64_t *)&header_comp_data.header_names_aligned8[0];

                                        while (values != 0)
                                        {
                                            uint64_t checkValue = vals[(offsets & 0xFF)];
                                            if (checkValue == val)
                                            {
                                                header_idx = (values & 0xFF);
                                                offsets = 0;
                                                break;
                                            }

                                            offsets >>= 8;
                                            values >>= 8;
                                        }
                                    }
                                }
                            }

                            printf("HDR-IDX: %d < %d\n", header_idx, NUM_HEADER_HANDLERS);

                            if (header_idx < NUM_HEADER_HANDLERS)
                            {
#if 0
                                uint64_t bit = (0x1ul) << header_idx;
                                if ((bit & keep_headers) != 0)
                                {
                                    printf("KEEP!\n");
                                }
                                else
                                {
                                    printf("IGNORE!\n");
                                }
#endif
                            }

                            // uint32_t val = (uint32_t)(pathStart - pHeader);

#if SKIP_SECOND_HOST_ENTRY_CHECK != 1
                            // printf("PRS:%s|\n", pHeader);
                            if (HOST_CHECK == header_name_buffer.ui64[0])
                            {
                                // 400 Bad Request, more than one Host
                                headerState = 2;
                                break;
                            }
#endif
                        }
                        *pSep = ':';
                        *pathStart = '\r';
                    }
                    pathStart += 2;
                    ++i;
                }
                else
                {
                    pathStart++;
                    ++i;
                }
            }

            if (headerState == 1)
            {
                printf("Parse error.\n");
                closeCC(pCC);
                return 0;
            }

            if (hasHost == 0)
            {
                printf("NO HOST!\n");
                closeCC(pCC);
                return 0;
            }
            INC_STATE_OR_SET(state, PARSE_STATE_REQ_HEADERS);
            SET_PARSE_STATE(pCC, state);
        }
        // parse/collect content
        //- check if content there, but forbidden.
        if (state == PARSE_STATE_REQ_HEADERS)
        {
            //
            printInfo(PARSE_CONTENT_CODE, 0, 0);
        }

        SET_PARSE_STATE(pCC, state);

        // parse content, if needed

        if (strlen(pStart) == 0 || strcmp("/", pStart) == 0)
        {
            sleep(1);
            pStart = "index.html";
        }

        int resSize = 0;
        void *cacheData = searchPathInCache(pStart, &resSize);

        if (cacheData != 0 && resSize != 0)
        {
            int s =
                send(clientSocketId, cacheData, resSize, 0);
            printf("SENT:C: %d | %d\n", s, resSize);
        }
        else
        {
            char *data = "HTTP/1.0 404 Not Found\r\nConnection: close\r\n\r\n";
            // int s =
            send(clientSocketId, data, strlen(data), 0);
            printf("NOT FOUND!\n");
        }
    }

    closeCC(pCC);
    return RECV_PARSE_NO_ERROR;
}

#include "http_parser_test.h"

/*
// GET /file ....

char* file_request = buffer + 5;
char response[1024] = {0};
char* metadata = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";

memcpy(response, metadata, strlen(metadata));

if (strncmp(file_request, "index.html ", 11) == 0) {
FILE* f = fopen("index.html", "r");
fread(response + strlen(metadata), 1024 - strlen(metadata) - 1, 1, f);
fclose(f);
} else {
char* error = "No page found";
memcpy(response + strlen(metadata), error, strlen(error));
}

SSL_write(ssl, response, 1024);
SSL_shutdown(ssl);
*/

#endif

#if 0
                // find separator
                while (i < recvRes && *pathStart != '\n')
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
                    ++i;
                }
#endif