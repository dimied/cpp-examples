#ifndef HTTP_PARSER_ASSIGNED_DATA
#define HTTP_PARSER_ASSIGNED_DATA

// code-replace:error505
#define ERROR_HTML_CONTENT_505 "<!DOCTYPE html><html lang=\"en\"><head><title>505 HTTP Version Not Supported</title></head></html>"

static const char error505[] = "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Type: text/html;\r\nContent-Length: 97\r\n\r\n" ERROR_HTML_CONTENT_505;

#define HTTP_ERROR_505_LEN (sizeof(error505))
// code-replace-end:error505

#define ERROR_HTTP_HEAD_500 "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html;\r\nContent-Length: 135\r\n\r\n"
#define ERROR_HTML_CONTENT_500 "<!doctype html><html lang=\"en\"><head><title>500 Internal Server Error</title></head><body><h1>Internal Server Error</h1></body></html>"
#define ERROR_HTML_CONTENT_500_LEN (sizeof(ERROR_HTML_CONTENT_500))

static const char error500[] = ERROR_HTTP_HEAD_500 "" ERROR_HTML_CONTENT_500;
#define HTTP_ERROR_500_LEN (sizeof(error500))

#endif