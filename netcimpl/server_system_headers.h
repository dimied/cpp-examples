#ifndef SERVER_SYSTEM_HEADERS_INCLUDED
#define SERVER_SYSTEM_HEADERS_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// funcs: sleep
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
// getprotobyname
// #include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#ifdef SSL_ENABLED
#if SSL_ENABLED
#include <openssl/ssl.h>
#endif
#endif

#include <openssl/err.h>

#endif