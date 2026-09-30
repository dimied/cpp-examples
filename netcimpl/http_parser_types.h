#ifndef HTTP_PARSER_TYPES_INCLUDED
#define HTTP_PARSER_TYPES_INCLUDED

#define PROTOCOL_MAJOR_VERSION(M) ((M >> 12) & 0x7)
#define PROTOCOL_MINOR_VERSION(M) ((M >> 8) & 0xF)

#ifdef HTTP_REQUEST_HAS_HOST
#error "HTTP_REQUEST_HAS_HOST predefined"
#endif

#ifdef HTTP_REQUEST_HAS_DIRECT_PATH
#error "HTTP_REQUEST_HAS_DIRECT_PATH predefined"
#endif

#ifdef HTTP_REQUEST_HAS_PARSED_HEADERS
#error "HTTP_REQUEST_HAS_PARSED_HEADERS predefined"
#endif

#define HTTP_REQUEST_HAS_HOST (0x1 << 16)
#define HTTP_REQUEST_FLAG_DIRECT_PATH (0x1 << 17)

#define HTTP_REQUEST_HAS_DIRECT_PATH(S) ((S & HTTP_REQUEST_FLAG_DIRECT_PATH) != 0)

#define HTTP_REQUEST_HAS_NO_DIRECT_PATH(S) ((S & HTTP_REQUEST_FLAG_DIRECT_PATH) == 0)

#define HTTP_REQUEST_HAS_PARSED_HEADERS (0x1 << 18)

#define HTTP_REQUEST_IS_PART_OF_CONNECTION (0x1 << 30)

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

#ifdef HTTP_REQUEST_DIRECT_PATH_LEN
#if HTTP_REQUEST_DIRECT_PATH_LEN < 8
#error "HTTP_REQUEST_DIRECT_PATH_LEN is less than 8"
#endif
#else
#define HTTP_REQUEST_DIRECT_PATH_LEN 8
#endif

#ifdef HTTP_REQUEST_DIRECT_CONTENT_LEN
#if HTTP_REQUEST_DIRECT_CONTENT_LEN < 8
#error "HTTP_REQUEST_DIRECT_CONTENT_LEN is less than 8"
#endif
#else
#define HTTP_REQUEST_DIRECT_CONTENT_LEN 8
#endif

#ifndef HTTP_REQUEST_PER_REQUEST_HEADERS_ENABLED
#define HTTP_REQUEST_PER_REQUEST_HEADERS_ENABLED 0
#endif

typedef struct HttpRequest
{
    // Bits:0-7:for request method, (if bit set, then custom method)
    // Bits:8-11: HTTP minor version
    // Bits:12-15: HTTP major version
    // Bits:16: flag - has host
    // Bits:17: flag - direct path (i.e. no pointer to path, path is stored directly as chars)
    // Bits:18: flag - headers parsed (at least partially)
    // Bits:18: flag -  ...
    // Bits:24-29: custom
    // Bits:28-29: connection (0:unknown, 2: close, 1: keep-alive, )
    // Bits:30: part of client connection (i.e. no free required)
    unsigned int requestInfo;
    // Bits:0-7: real state
    // Bits:8... custom
    unsigned int __state;
#if HTTP_REQUEST_PER_REQUEST_HEADERS_ENABLED != 0
    ui64_type accepted_headers;     /*< collection(bitmask) of header to collect */
    ui64_type accepted_headers_chs; /*< collection(bitmask) of header to collect */
#endif
    ui64_type current_headers;     /*< collection(bitmask) of headers collected */
    ui64_type current_headers_chs; /*< collection(bitmask) of header collected */
    ui64_type __validation;        /*!< e.g. for validating connection */
    union
    {
        /*@null@*/ char *ptr;
        char text[HTTP_REQUEST_DIRECT_PATH_LEN];
    } path;
#if HTTP_REQUEST_PART_OF_CONNECTION_TYPE == 0
    /*@null@*/ void *__clientConnection;
#endif
    /*@null@*/ void *__headers;
    union
    {
        /*@null@*/ void *ptr;
        char direct[HTTP_REQUEST_DIRECT_CONTENT_LEN];
    } __content;
    union
    {
        /*@null@*/ void *ptr;
        char direct[8];
        unsigned long ulong_value;
        long long_value;
    } custom1;
} HttpRequest;

#endif