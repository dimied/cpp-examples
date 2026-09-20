#ifndef HTTP_PARSER_DEV_LOG_INCLUDED
#define HTTP_PARSER_DEV_LOG_INCLUDED

#define RECV_CODE 1
#define METHOD_CODE 2
#define METHOD_INVALID_CODE 3
#define PATH_CODE 4
#define PARSE_HEADERS_CODE 5
#define PARSE_CONTENT_CODE 10

#if HTTP_PARSER_DEV_LOG
void printInfo(int code, char *data, ssize_t val)
{
    switch (code)
    {
    case RECV_CODE:
        printf("RECV: %ld\n", val);
        return;
    case METHOD_CODE:
        printf("METHOD: %s|%ld\n", data, val);
        return;
    case METHOD_INVALID_CODE:
        printf("Invalid method found! (%s)\n", data);
        return;
    case PATH_CODE:
        printf("PATH:%s|(#%ld)\n", data, val);
        return;
    case PARSE_HEADERS_CODE:
        printf("PARSE HEADERS! #%ld\n", NUM_HEADER_HANDLERS);
        return;
    case PARSE_CONTENT_CODE:
        printf("PARSE CONTENT\n");
        return;
    default:
        printf("Unknown: %d|%p|%ld\n", code, data, val);
    }
}
#else
#define printInfo(CODE, DATA, VAL)
#endif

#endif