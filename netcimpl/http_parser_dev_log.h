#ifndef HTTP_PARSER_DEV_LOG_INCLUDED
#define HTTP_PARSER_DEV_LOG_INCLUDED

#define RECV_CODE 1
#define METHOD_CODE 2
#define METHOD_INVALID_CODE 3
#define PATH_CODE 4
#define PARSE_HEADERS_CODE 5
#define PARSE_CONTENT_CODE 10

#if HTTP_PARSER_DEV_LOG
void printInfo(int code, /*@null@*/ char *data, int val)
{
    /*@notnull@*/ const char *pD = (data != (char *)0) ? (const char *)data : "nil";

    switch (code)
    {
    case RECV_CODE:
        printf("RECV: %d\n", val);
        return;
    case METHOD_CODE:
        /*@-nullpass@*/
        printf("METHOD: %s|%d\n", pD, val);
        /*@+nullpass@*/
        return;
    case METHOD_INVALID_CODE:
        /*@-nullpass@*/
        printf("Invalid method found! (%s)\n", pD);
        /*@+nullpass@*/
        return;
    case PATH_CODE:
        /*@-nullpass@*/
        printf("PATH:%s|(#%d)\n", pD, val);
        /*@+nullpass@*/
        return;
    case PARSE_HEADERS_CODE:
        printf("PARSE HEADERS! #%d\n", NUM_HEADER_HANDLERS);
        return;
    case PARSE_CONTENT_CODE:
        printf("PARSE CONTENT\n");
        return;
    default:
        printf("Unknown: %d|%p|%d\n", code, data, val);
    }
}
#else
#define printInfo(CODE, DATA, VAL)
#endif

#endif