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

#ifndef HTTP_PARSER_MACROS
#if HTTP_PARSER_DECLARATIONS
#define HTTP_PARSER_MACROS 1
#endif
#endif

#if HTTP_PARSER_MACROS
#define WORD_ES(BC) ((unsigned long)' ' << BC)

#define WORD3_TO_UINT64(C1, C2, C3) (C1 | (C2 << 8) | (C3 << 16))
#define WORD3_TO_UINT64_ES_END(C1, C2, C3) (WORD3_TO_UINT64(C1, C2, C3) | WORD_ES(24))
#define WORD4_TO_UINT64(C1, C2, C3, C4) (WORD3_TO_UINT64(C1, C2, C3) | (C4) << 24)
#define WORD4_TO_UINT64_ES_END(C1, C2, C3, C4) (WORD4_TO_UINT64(C1, C2, C3, C4) | WORD_ES(32))
#define WORD5_TO_UINT64_ES_END(C1, C2, C3, C4, C5) (WORD3_TO_UINT64(C1, C2, C3) | (C4 << 24) | ((unsigned long)C5 << 32) | WORD_ES(40))
#define WORD6_TO_UINT64_ES_END(C1, C2, C3, C4, C5, C6) (WORD3_TO_UINT64(C1, C2, C3) | (C4 << 24) | ((unsigned long)C5 << 32) | ((unsigned long)C6 << 40) | WORD_ES(48))
#define WORD7_TO_UINT64_ES_END(C1, C2, C3, C4, C5, C6, C7) (WORD3_TO_UINT64(C1, C2, C3) | (C4 << 24) | ((unsigned long)C5 << 32) | ((unsigned long)C6 << 40) | ((unsigned long)C7 << 48) | WORD_ES(56))

#define WORD8_TO_UINT64(C1, C2, C3, C4, C5, C6, C7, C8) (WORD3_TO_UINT64(C1, C2, C3) | (C4 << 24) | ((unsigned long)C5 << 32) | ((unsigned long)C6 << 40) | ((unsigned long)C7 << 48) | ((unsigned long)C8 << 56))
#endif

#ifndef HTTP_PARSER_CONSTANTS
#define HTTP_PARSER_CONSTANTS

#define MIME_TYPE_HTML 1
#define MIME_TYPE_XHTML_XML 2
#define MIME_TYPE_XML 3

#define MIME_TYPE_IMAGE_ANY 126
#define MIME_TYPE_ANY 127
#endif

#ifdef HTTP_PARSER_DECLARATIONS
#if HTTP_PARSER_DECLARATIONS
typedef struct RequestHttpHeaders
{
    unsigned short acceptedTypes[4]; // pairs (quality,type)
} RequestHttpHeaders;

#ifndef HTTP_REQUEST_NUM_PTRS
#define HTTP_REQUEST_NUM_PTRS 4
#endif

#define PROTOCOL_MAJOR_VERSION(M) ((M >> 12) & 0x7)
#define PROTOCOL_MINOR_VERSION(M) ((M >> 8) & 0xF)

#define HTTP_REQUEST_RI_HAS_HOST (0x1 << 16)
#define HTTP_REQUEST_RI_HAS_PARSED_HEADERS (0x1 << 17)

#define ADD_HEADERS_OK 0
#define ADD_HEADERS_INVALID_ARGS 1
#define ADD_HEADERS_NOMEM 2
#define ADD_HEADERS_TOO_LARGE 3

typedef struct HeadersDataHeader
{
    void *next;
    unsigned int size_and_used;
    unsigned int options;
} HeadersDataHeader;

typedef struct HeadersDataInfo
{
    unsigned int size;
    unsigned int used;
    unsigned int num_chunks;
} HeadersDataInfo;

#define HTTP_REQUEST_STATE_PARSING 1

typedef struct HttpRequest
{
    // Bits:0-7:for request method
    // Bits:8-11: HTTP minor version
    // Bits:12-15: HTTP major version
    // Bits:16: flag - has host
    // Bits:17: flag - headers parsed (at least partially)
    // Bits:18: flag -  ...
    // Bits:24-29: custom
    // Bits:30-31: connection (0:unknown, 2: close, 1: keep-alive, )
    unsigned int requestInfo;
    // Bits:0-7: real state
    // Bits:8... custom
    unsigned int __state;
    uint64_t __validation; /*!< e.g. for validating connection */
    char *path;
    void *__clientConnection;
    uint64_t acceptedHeaders; /*< collection(bitmask) of header to collect */
    void *__headers;
    void *__content;
    void *pCustom1;
} HttpRequest;

void clearHeadersDataInfo(HeadersDataInfo *pInfo)
{
    if (pInfo != 0)
    {
        memset(pInfo, 0, sizeof(HeadersDataInfo));
    }
}

void printHeadersDataInfo(HeadersDataInfo *pInfo)
{
    if (pInfo != 0)
    {
        printf("HI: size:%d | used:%d | chunks:%d\n", pInfo->size, pInfo->used, pInfo->num_chunks);
    }
    else
    {
        printf("HI: (nil)\n");
    }
}

int getHeadersDataInfo(HttpRequest *pReq, HeadersDataInfo *pInfo)
{
    if (pReq != 0 && pInfo != 0)
    {
        void *headers = pReq->__headers;
        if (headers != 0)
        {
            unsigned int size = 0, used = 0, chunks = 0;

            while (headers != 0)
            {
                HeadersDataHeader *pH = (HeadersDataHeader *)headers;
                used += pH->size_and_used & 0xFFFF;
                size += pH->size_and_used >> 16;
                ++chunks;
                headers = pH->next;
            }
            pInfo->size = size;
            pInfo->used = used;
            pInfo->num_chunks = chunks;
        }
        return 0;
    }
    return 1;
}

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

//TODO:instead of saving full header name, 
// we can save (0x80 | HEADER_IDX) as first byte and reducing the amount of bytes
//for unknown headers we save full header name.
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

#define REQ_METHOD_SUPPORT_CONNECT 0x1
#define REQ_METHOD_SUPPORT_DELETE 0x2
#define REQ_METHOD_SUPPORT_GET 0x4
#define REQ_METHOD_SUPPORT_HEAD 0x8

#define REQ_METHOD_SUPPORT_OPTIONS 0x10
#define REQ_METHOD_SUPPORT_PATCH 0x20
#define REQ_METHOD_SUPPORT_POST 0x40
#define REQ_METHOD_SUPPORT_PUT 0x80
#define REQ_METHOD_SUPPORT_TRACE 0x100

/*
GET: The GET method requests a representation of the specified resource. Requests using GET should only retrieve data and should not contain a request content.
HEAD: The HEAD method asks for a response identical to a GET request, but without a response body.
POST: The POST method submits an entity to the specified resource, often causing a change in state or side effects on the server.
PUT: The PUT method replaces all current representations of the target resource with the request content.
DELETE: The DELETE method deletes the specified resource.
CONNECT: The CONNECT method establishes a tunnel to the server identified by the target resource.
OPTIONS: The OPTIONS method describes the communication options for the target resource.
TRACE: The TRACE method performs a message loop-back test along the path to the target resource.
PATCH: The PATCH method applies partial modifications to a resource.
*/
#define NUM_HTTP_METHODS 9

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

char *HttpMethods[NUM_HTTP_METHODS] = {
    "CONNECT",
    "DELETE",
    "GET",
    "HEAD",
    "OPTIONS",
    "PATCH",
    "POST",
    "PUT",
    "TRACE",
};

#if 1
uint64_t HttpMethods2[NUM_HTTP_METHODS] = {
    WORD7_TO_UINT64_ES_END('C', 'O', 'N', 'N', 'E', 'C', 'T'),
    WORD6_TO_UINT64_ES_END('D', 'E', 'L', 'E', 'T', 'E'),
    WORD3_TO_UINT64_ES_END('G', 'E', 'T'),
    WORD4_TO_UINT64_ES_END('H', 'E', 'A', 'D'),
    WORD7_TO_UINT64_ES_END('O', 'P', 'T', 'I', 'O', 'N', 'S'),
    WORD5_TO_UINT64_ES_END('P', 'A', 'T', 'C', 'H'),
    WORD4_TO_UINT64_ES_END('P', 'O', 'S', 'T'),
    WORD3_TO_UINT64_ES_END('P', 'U', 'T'),
    WORD5_TO_UINT64_ES_END('T', 'R', 'A', 'C', 'E'),
};
#endif
#endif

#ifdef HTTP_PARSER_IMPLEMENTATION
#if HTTP_PARSER_IMPLEMENTATION

// TODOs: accept empty spaces

#endif
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

#if 0
#define LENGTH_INDEX_MASK3 ((unsigned long)(REQ_METHOD_SUPPORT_GET | REQ_METHOD_SUPPORT_PUT))
#define LENGTH_INDEX_MASK4 ((unsigned long)(REQ_METHOD_SUPPORT_HEAD | REQ_METHOD_SUPPORT_POST))
#define LENGTH_INDEX_MASK5 ((unsigned long)(REQ_METHOD_SUPPORT_PATCH)
#define LENGTH_INDEX_MASK6 ((unsigned long)(REQ_METHOD_SUPPORT_DELETE)
#define LENGTH_INDEX_MASK7 ((unsigned long)(REQ_METHOD_SUPPORT_CONNECT|REQ_METHOD_SUPPORT_OPTIONS)
// Warning: 'TRACE' excluded
#define LENGTH_INDEX_MASK ((LENGTH_INDEX_MASK3 << 24) | (LENGTH_INDEX_MASK4 << 32) | (LENGTH_INDEX_MASK5 << 40) | (LENGTH_INDEX_MASK5 << 48) | (LENGTH_INDEX_MASK5 << 56))
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
        char *s = noneString;

        int m = p->requestInfo & 0x1F;
        if (m < 9)
        {
            s = HttpMethods[m];
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
                    if (0 == strcmp(HttpMethods[j], inBuffer.data))
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

#define HO_LEN 32
            uint32_t headerOffsets[HO_LEN];
            size_t pIdx = 0;
            int hasHost = 0;
            // char *pHeadersBefore = pathStart;
            int headerState = 0;
            // TODO: Check i and recvRes
            int hIdx = 0;

            while (headerState == 0 && recvRes - i >= 2 && *pathStart != '\r' && pathStart[1] != '\n')
            {
                char *pHeader = pathStart;
                char *pSep = (char *)0;
                printf("READ:HEADER:%d\n", hIdx);
                hIdx++;

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

                if (headerState != 0)
                {
                    printf("INVALID CHAR!\n");
                    break;
                }

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
                        printf("H|%s||%s|\n", pHeader, pSep + 1);

                        if (hasHost)
                        {
                            // printf("PRS:%s|\n", pHeader);
                            if (strcmp(pHeader, "host") == 0)
                            {
                                // 400 Bad Request, more than one Host
                                return 0;
                            }
                        }
                        else
                        {
                            if (strcmp(pHeader, "host") == 0)
                            {
                                printf("HOST!%s\n", pSep + 1);
                                int hostRes = isValidHost(pCC, pSep + 1);
                                if (IS_VALID_HOST_OK != hostRes)
                                {
                                    // 400 Bad Request
                                    return 0;
                                }
                                hasHost++;
                            }
                            else
                            {
                                if (pIdx < HO_LEN)
                                {
                                    uint32_t val = (uint32_t)(pathStart - pHeader);
                                    if (val >= 0x1000000)
                                    {
                                        val = 0;
                                    }
                                    headerOffsets[pIdx] = val;
                                    pIdx++;
                                }
                            }
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