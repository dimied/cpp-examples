#ifndef HTTP_PARSER
#define HTTP_PARSER

#ifdef HTTP_PARSER_DECLARATIONS
#if HTTP_PARSER_DECLARATIONS

typedef struct RequestHttpHeaders
{
    unsigned short acceptedTypes[4]; // pairs (quality,type)
} RequestHttpHeaders;

typedef struct RequestHttpContent
{
    void *data;
} RequestHttpContent;

#define HTTP_REQUEST_BUF_LEN 32

#ifdef HTTP_REQUEST_NUM_PTRS
#if HTTP_REQUEST_NUM_PTRS != 4 || HTTP_REQUEST_NUM_PTRS != 8
#error "HTTP_REQUEST_NUM_PTRS not 4 or 8"
#endif
#else
#define HTTP_REQUEST_NUM_PTRS 4
#endif

#define PROTO_MAJOR_VERSION(M) ((M >> 11) & 0x7)
#define PROTO_MINOR_VERSION(M) ((M >> 8) & 0x7)

typedef struct HttpRequest
{
    // 5 bits for request method
    // 3 bits reserved for custom request method.
    // 3 bits for major http version, 3 bits for minor http version
    // 2 bits reserved
    unsigned int requestMethod;
    unsigned int state;

#if 0
    uint64_t pointerTypes; 
    void* pointers[8];
#else
    uint32_t pointerTypes;
    // last ptr should be used for current parsing.
    void *pointers[HTTP_REQUEST_NUM_PTRS];
#endif
    char *path;
    RequestHttpHeaders headers;
    RequestHttpContent content;
} HttpRequest;

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
            free(pReq->path);
            pReq->path = 0;
        }
        free(p);
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

typedef struct UnknownHeaderResult
{
    size_t capacity;
    void **values;
    size_t size;
} UnknownHeaderResult;
#endif
#endif

static const struct table_entry
{
    const char *extension;
    const char *content_type;
} content_type_table[] = {
    //{"c", "text/plain"},
    //{"h", "text/plain"},
    {"css", "text/css"},
    {"gif", "image/gif"},
    {"htm", "text/htm"},
    {"html", "text/html"},
    {"ico", "image/vnd.microsoft.icon"},
    {"jpeg", "image/jpeg"},
    {"jpg", "image/jpeg"},
    {"js", "text/javascript"},
    {"json", "application/json"},
    {"svg", "image/svg+xml"},
    {"png", "image/png"},
    {"pdf", "application/pdf"},
    {"ps", "application/postscript"},
    {"txt", "text/plain"}};

#include "mimetypecreate.h"

#ifdef HTTP_PARSER_IMPLEMENTATION
#if HTTP_PARSER_IMPLEMENTATION

/*
   {
    "requestHeaders": {
        "headers": [
{
"name": "Accept-Encoding",
"value": "gzip, deflate, br, zstd"
},
{
"name": "Accept-Language",
"value": "de,en-US;q=0.9,en;q=0.8"
},
{
"name": "Cache-Control",
"value": "no-cache"
},
{
"name": "Connection",
"value": "keep-alive"
},
{
"name": "DNT",
"value": "1"
},
{
"name": "Host",
"value": "www.dimaum.de"
},
{
"name": "Pragma",
"value": "no-cache"
},
{
"name": "Priority",
"value": "u=0, i"
},
{
"name": "Sec-Fetch-Dest",
"value": "document"
},
{
"name": "Sec-Fetch-Mode",
"value": "navigate"
},
{
"name": "Sec-Fetch-Site",
"value": "none"
},
{
"name": "Sec-Fetch-User",
"value": "?1"
},
{
"name": "Sec-GPC",
"value": "1"
},
{
"name": "Upgrade-Insecure-Requests",
"value": "1"
},
{
"name": "User-Agent",
"value": "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:155.0) Gecko/20100101 Firefox/155.0"
}
]
}
}
*/

typedef struct AcceptType
{
    unsigned int size;
    unsigned int type;
    const char *str;
} AcceptType;

#define ENTRY_AcceptType(T, S) {(unsigned int)sizeof(S), T, S}

#define MIME_TYPE_HTML 1
#define MIME_TYPE_XHTML_XML 2
#define MIME_TYPE_XML 3

#define MIME_TYPE_IMAGE_ANY 126
#define MIME_TYPE_ANY 127

// TODOs: accept empty spaces

// to speed up decide by first char (range: '*' - 'z')
AcceptType acceptTypes[] = {
    ENTRY_AcceptType(MIME_TYPE_ANY, "*/*"),
    ENTRY_AcceptType(MIME_TYPE_XHTML_XML, "application/xhtml+xml"),
    ENTRY_AcceptType(MIME_TYPE_XML, "application/xml"),
    ENTRY_AcceptType(MIME_TYPE_HTML, "text/html"),
    ENTRY_AcceptType(MIME_TYPE_IMAGE_ANY, "image/*"),
};

#define NUM_ACCEPT_TYPES (sizeof(acceptTypes) / sizeof(acceptTypes[0]))

#define UNKNOWN_TYPE_OFFSET (128)
#define MAX_RECOGNITION_TYPES 32
#define MAX_UNKNOWN_TYPES 32

#ifndef COLLECT_UNKNOWN_TYPES
#define COLLECT_UNKNOWN_TYPES 1
#endif

#ifndef PARSE_ACCEPT_PARAM_CHECK
#define PARSE_ACCEPT_PARAM_CHECK 0
#endif

void parseAccept(char *pAccept, int len, unsigned short *pAcceptRes, int acceptResLen, UnknownHeaderResult *pU)
{
#ifdef COLLECT_UNKNOWN_TYPES
#if COLLECT_UNKNOWN_TYPES
#define DO_COLLECT_UNKNOWN_TYPES 1
#else
#define DO_COLLECT_UNKNOWN_TYPES 0
#endif
#else
#define DO_COLLECT_UNKNOWN_TYPES 0
#endif

#if PARSE_ACCEPT_PARAM_CHECK
    if (pAcceptRes == 0 || acceptResLen <= 0)
    {
        return 0;
    }
#endif
    char *pStart = pAccept;
    char *pEnd = pAccept + len;
    char *pQ = pEnd;
    unsigned int type = 0;
    int typesGot = 0, typeIdx = 0;
    unsigned int types[MAX_RECOGNITION_TYPES];

#if DO_COLLECT_UNKNOWN_TYPES
    int unknownTypesGot = 0;
    void *unknownTypes[MAX_UNKNOWN_TYPES];
#endif

    while (pAccept <= pEnd)
    {
        char val = *pAccept;

        if (pAccept == pEnd)
        {
            val = ',';
        }

        if (val == '\r' || val == '\n')
        {
            // should be impossible
            break;
        }

        if (val == '/')
        {
            ++type; // should happen only once for valid types
        }
        else if (val == ';')
        {
            pQ = pAccept;
        }
        else if (val == ',')
        {
            long int diff = (pAccept - pStart);
            // at least 'asterisk/asterisk' and need one separator '/'
            if (type == 1 && diff > 2)
            {
                int q = 100;
                if (pQ > pAccept)
                {
                    pQ = pAccept;
                }
                else
                {
                    q = 0;
                }
                // parse type first
                diff = pQ - pStart;
                type = 0;

                if (diff < 120)
                {
                    // for (int i = 0; i < (int)NUM_ACCEPT_TYPES; i++)

                    {
                        int i;
                        for (i = 0; i < (int)NUM_ACCEPT_TYPES; i++)
                        {
                            if (acceptTypes[i].size == (unsigned int)diff)
                            {
                                if (0 == strncmp(pStart, acceptTypes[i].str, (size_t)diff))
                                {
                                    type = acceptTypes[i].type;
                                }
                            }
                        }
                    }

                    if (q == 0)
                    {
                        q = 50;
                        ++pQ;
                        if (*pQ == 'q' && pQ[1] == '=' && pQ[2] == '0' && pQ[3] == '.')
                        {
                            pQ += 4;

                            if (*pQ >= '0' && *pQ <= '9')
                            {
                                q = 10 * (int)((*pQ) - '0');
                                ++pQ;
                                if (*pQ >= '0' && *pQ <= '9')
                                {
                                    q += (int)((*pQ) - '0');
                                    // forget third decimal
                                }
                            }
                        }
                    }
                }

                if (type != 0)
                {
                    if (typesGot < MAX_RECOGNITION_TYPES)
                    {
                        types[typesGot] = ((unsigned)q << 8) | type;
                        ++typesGot;
                    }
                }
                else
                {
                    if (unknownTypesGot < MAX_UNKNOWN_TYPES)
                    {
                        unknownTypes[unknownTypesGot] = pStart;
                        ++unknownTypesGot;
                    }
                }
                ++typeIdx;
            }
            type = 0;
            pStart = pAccept + 1;
            pQ = pEnd;
        }
        ++pAccept;
    }
    if (typesGot > 0)
    {
        int i = 0;
        while (i < typesGot && acceptResLen > 0)
        {
            pAcceptRes[i] = (unsigned short)types[i];
            ++i;
            --acceptResLen;
        }
    }

#if DO_COLLECT_UNKNOWN_TYPES
    if (unknownTypesGot > 0 && pU != 0 && pU->size < pU->capacity && pU->values != 0)
    {
        int i = 0;
        while (i < unknownTypesGot && pU->size < pU->capacity)
        {
            pU->values[pU->size] = unknownTypes[i];
            ++i;
            pU->size++;
        }
    }
#endif

#undef DO_COLLECT_UNKNOWN_TYPES
}

void parseAcceptEncoding(char *pAcceptEncoding, unsigned long *pAcceptRes)
{
    // gzip, deflate, br, zstd
    (void)(pAcceptEncoding);
    (void)(pAcceptRes);
}

void parseAcceptLanguage(char *pAcceptLanguage, unsigned long *pAcceptRes)
{
    (void)(pAcceptLanguage);
    (void)(pAcceptRes);
}

#endif
#endif

#define BUF_LEN_IN 2048

#ifdef SET_STATES
#define INC_STATE_OR_SET(STATE, NEW_STATE) (STATE = NEW_STATE)
#else
#define INC_STATE_OR_SET(STATE, NEW_STATE) (STATE++)
#endif

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

#if 0
#define ES(BC) ((unsigned long)' ' << BC)

#define WORD3_TO_UINT64(C1, C2, C3) (C1 | (C2 << 8) | (C3 << 16) | ES(24))
#define WORD4_TO_UINT64(C1, C2, C3, C4) (WORD3_TO_UINT64(C1, C2, C3) | (C4 << 24) | ES(32))
#define WORD5_TO_UINT64(C1, C2, C3, C4, C5) (WORD4_TO_UINT64(C1, C2, C3, C4) | ((unsigned long)C5 << 32) | ES(40))
#define WORD6_TO_UINT64(C1, C2, C3, C4, C5, C6) (WORD5_TO_UINT64(C1, C2, C3, C4, C5) | ((unsigned long)C6 << 40) | ES(48))
#define WORD7_TO_UINT64(C1, C2, C3, C4, C5, C6, C7) (WORD6_TO_UINT64(C1, C2, C3, C4, C5, C6) | ((unsigned long)C7 << 48) | ES(56))

uint64_t HttpMethods2[NUM_HTTP_METHODS] = {
    WORD7_TO_UINT64('C', 'O', 'N', 'N', 'E', 'C', 'T'),
    WORD6_TO_UINT64('D', 'E', 'L', 'E', 'T', 'E'),
    WORD3_TO_UINT64('G', 'E', 'T'),
    WORD4_TO_UINT64('H', 'E', 'A', 'D'),
    WORD7_TO_UINT64('O', 'P', 'T', 'I', 'O', 'N', 'S'),
    WORD5_TO_UINT64('P', 'A', 'T', 'C', 'H'),
    WORD4_TO_UINT64('P', 'O', 'S', 'T'),
    WORD3_TO_UINT64('P', 'U', 'T'),
    WORD5_TO_UINT64('T', 'R', 'A', 'C', 'E'),
};
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
char *parseStates[] = {
    "Line",
    "Method",
};
char *noneString = "(nil)";

void printRequest(HttpRequest *p)
{
    if (p != 0)
    {
        printf("Req:\n");
        char *s = noneString;

        int m = p->requestMethod & 0x1F;
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
        int major = PROTO_MAJOR_VERSION(p->requestMethod), minor = PROTO_MINOR_VERSION(p->requestMethod);
        printf(">PROTO:%d.%d\n", major, minor);
        printf("\n");
    }
    else
    {
        printf("Req: (nil)\n");
    }
}

int basicHandleClientConnection(void *pClientConnection, void *pServerConnection)
{
    (void)(pServerConnection);
    MyServerClientConnection *pCC = (MyServerClientConnection *)pClientConnection;

    union
    {
        uint64_t data64[(BUF_LEN_IN + 7) / 8];
        char data[BUF_LEN_IN];
    } inBuffer;

    int clientSocketId = pCC->socketId;
    if (clientSocketId)
    {
        ssize_t recvRes;
        // memset(buffer, 0, BUF_LEN_IN);

        printf("PARSE-STATE: %d\n", pCC->parseState);

        if (pCC->state & 0x100)
        {
            MyServerClientConnectionSecure *pSC = (MyServerClientConnectionSecure *)pCC;

            //<=0 means error
            recvRes = SSL_read(pSC->ssl, inBuffer.data, BUF_LEN_IN - 1);
        }
        else
        {
            recvRes = recv(clientSocketId, inBuffer.data, BUF_LEN_IN - 1, 0);
        }

        printf("RECV: %ld\n", recvRes);
        printf("Request:\n----\n%s\n----\n", inBuffer.data);

        if (recvRes <= 0)
        {
            // TODO:check EAGAIN or EWOULDBLOCK.
            return 0;
        }
        int state = pCC->parseState;
        char *pathStart = inBuffer.data;
        int i = 1;
        ++recvRes;

        // Strategy:
        //- use a buffer and collect data
        //- for every state we can define minimal amount of bytes required
        //   - for parsing content we refer to the method
        //   - e.g. GET has no data, POST can have some data

        HttpRequest *pReq = pCC->request;

        if (PARSE_STATE_REQ_LINE == state)
        {
            // We need to be able to parse at least 7 chars and ' '
            if (recvRes < 8)
            {
                closeCC(pCC);
                return 0;
            }
            if (pReq == 0)
            {
                pReq = newRequest();

                if (pReq == 0)
                {
                    printf("REQ not created\n");
                    closeCC(pCC);
                    return 0;
                }
                pCC->request = pReq;
            }

            printf("PARSE METHOD\n");

            int s = 0;
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
                closeCC(pCC);
                return 0;
            }
            else
            {
                *pathStart = 0;
            }
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
                printf("METHOD: %s|%d\n", inBuffer.data, j);
                pReq->requestMethod = j;
            }
            else
            {
                printf("Invalid method found! (%s)\n", inBuffer.data);
                closeCC(pCC);
                return 0;
            }
            ++pathStart;
            //++i;
            INC_STATE_OR_SET(state, PARSE_STATE_REQ_LINE_HAS_METHOD);
        }

        char *pStart = pathStart;
        // parse path
        if (state == PARSE_STATE_REQ_LINE_HAS_METHOD)
        {
            printf("PARSE PATH\n");
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
                closeCC(pCC);
                return 0;
            }
            else
            {
                // TODO: long paths
                *pathStart = 0;
            }
            ++pathStart;

            printf("PATH:%s|(#%ld)\n", pStart, diff);
            char *pReqPath = memoryForPath(diff);
            if (pReqPath == 0)
            {
                closeCC(pCC);
                return 0;
            }
            memcpy(pReqPath, pStart, diff);
            pReq->path = pReqPath;
            INC_STATE_OR_SET(state, PARSE_STATE_REQ_LINE_PATH);
        }

        printRequest(pReq);

        // min-size: 6 bytes
        if (state == PARSE_STATE_REQ_LINE_PATH)
        {
            printf("PARSE PROT\n");
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
                if (*pathStart == ' ')
                {
                    break;
                }
                ++pOK;
                ++pathStart;
                ++i;
            }
            if (i < 0 || pOK < 6 || pOK >= 10)
            {
                printf("CLOSE! %d|%d\n", i, pOK);
                closeCC(pCC);
                return 0;
            }
            else
            {
                int major, minor;
                *pathStart = 0;
                int numItems = sscanf(protStart, "HTTP/%d.%d", &major, &minor);
                printf("PROT:%s|M=%d,m=%d\n", protStart, major, minor);
                if (numItems != 2 || major < 0 || major > 2 || minor < 0 || minor > 7)
                {
                    closeCC(pCC);
                    return 0;
                }
                minor |= major << 3;

                pReq->requestMethod |= ((unsigned int)minor << 8);

                pathStart += 2;
                ++i;
            }

            state++;
        }
        printRequest(pReq);

        // parsing separator line
        if (state == 3)
        {
            printf("PARSE SEP-LINE\n");
            int ok = 0;
            if (recvRes - i >= 2)
            {
                if (*pathStart == '\r' && pathStart[1] == '\n')
                {
                    ok++;
                    pathStart += 2;
                    i += 2;
                }
            }
            else
            {
                closeCC(pCC);
                return 0;
            }
            if (ok)
            {
                state++;
                ok--;
            }
        }

        // parsing headers
        if (state == 4)
        {
            printf("PARSE HEADERS\n");
            if (recvRes - i >= 2)
            {
                if (*pathStart == '\r' && pathStart[1] == '\n')
                {
                    // ok++;
                    pathStart += 2;
                    i += 2;
                    state++;
                }
                else
                {
                }
            }
        }
        // parse/collect content
        //- check if content there, but forbidden.
        if (state == 5)
        {
            //
            printf("PARSE CONTENT\n");
        }

        pCC->parseState = state;

        // parse content, if needed

        if (strlen(pStart) == 0 || strcmp("/", pStart) == 0)
        {
            printf("DEFAULT\n");
            sleep(1);
            pStart = "index.html";
        }

        int resSize = 0;
        void *cacheData = searchPathInCache(pStart, &resSize);

        printf("FROM CACHE! %p\n", cacheData);

        if (cacheData != 0 && resSize != 0)
        {
            int s = send(clientSocketId, cacheData, resSize, 0);
            printf("SENT:C: %d | %d\n", s, resSize);
        }
        else
        {
            char *data = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\nHallo";
            char *data2 = "HTTP/1.0 200 OK\r\nContent-Type: image/x-icon\r\n\r\n";
            char *data3 = "HTTP/1.0 404 Not Found\r\n";

            char *p = strstr(pStart, "favicon.ico");
            if (p != 0)
            {
                int r = loadFAV();
                if (r == 0)
                {
                    if (favResponse != 0)
                    {
                        int s = send(clientSocketId, favResponse, favResponseLen, 0);
                        printf("SENT:R: %d\n", s);
                    }
                    else
                    {
                        int s = send(clientSocketId, data2, strlen(data2), 0);
                        printf("SENT:1: %d\n", s);
                        s = send(clientSocketId, favData, favDataLen, 0);
                        printf("SENT:2: %d\n", s);
                    }
                }
                else
                {
                    int s = send(clientSocketId, data3, strlen(data3), 0);
                    printf("SENT:ERR: %d\n", s);
                }
            }
            else
            {
                recvRes = send(clientSocketId, data, strlen(data), 0);
                printf("SENT: %ld\n", recvRes);
            }
        }

        closeCC(pCC);
        // sleep(5);

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
        // sleep(1);
    }

    return 0;
}
#endif