#ifndef HTTP_PARSER_TYPES_INCLUDED
#define HTTP_PARSER_TYPES_INCLUDED

#ifndef HTTP_REQUEST_NUM_PTRS
#define HTTP_REQUEST_NUM_PTRS 4
#endif

#define PROTOCOL_MAJOR_VERSION(M) ((M >> 12) & 0x7)
#define PROTOCOL_MINOR_VERSION(M) ((M >> 8) & 0xF)

#define HTTP_REQUEST_RI_HAS_HOST (0x1 << 16)
#define HTTP_REQUEST_RI_HAS_PARSED_HEADERS (0x1 << 17)

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
    /*@null@*/ char *path;
    void *__clientConnection;
    uint64_t acceptedHeaders; /*< collection(bitmask) of header to collect */
    /*@null@*/ void *__headers;
    /*@null@*/ void *__content;
    /*@null@*/ void *pCustom1;
} HttpRequest;

#endif