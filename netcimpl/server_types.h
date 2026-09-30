#ifndef SERVER_TYPES_INCLUDED
#define SERVER_TYPES_INCLUDED

#include "server_basic_types.h"

#ifndef HTTP_REQUEST_PART_OF_CONNECTION_TYPE
#define HTTP_REQUEST_PART_OF_CONNECTION_TYPE 0
#endif

/*!
@struct MyServerClientConnection
@brief Structure for a single connection (accepted by a server socket).
*/
typedef struct MyServerClientConnection
{
    int socketId;       /*!< socket for this connection */
    unsigned int state; /*!< stores current state of the connection.
                        Bits:0-7: for parser state.
                        Bits:8-15: connection state
                        Bits:16-23: errno
                         */
    uint64_t __validation;
    /*@null@*/ void *next; /*!< next pointer in the linked list*/
#if HTTP_REQUEST_PART_OF_CONNECTION_TYPE == 0
    /*@null@*/ void *request; /*!< request object */
#endif
    /*@null@*/ void *pServerConnection; /*!< pointer to server connection object */
    union
    {
#if USING_SSL
        /*@null@*/ SSL *ssl;
#endif
        /*@null@*/ void *reserved;
    } secure; /*!< used for security */
#if HTTP_REQUEST_PART_OF_CONNECTION_TYPE == 1
    HttpRequest request;
#endif
    union
    {
        char cbytes[CLIENT_CONNECTION_BUFFER_LEN];
    } buffer; /*!< some buffer */
} MyServerClientConnection;

typedef struct MyServerClientConnections
{
    size_t numCons;
    /*@only@*/ /*@null@*/ void *first;
    /*@shared@*/ /*@null@*/ void *last;
} MyServerClientConnections;

typedef struct SecureOptions
{
    SSL_CTX *ctx;
    char *certFilePath;
    char *privKeyFilePath;
    unsigned long lastError;
} SecureOptions;

typedef struct MyServerConnectionInfo
{
    int socketId;
    int status;
} MyServerConnectionInfo;

typedef int (*handleClientConnection)(void *pClientConnection, void *pServerConnection);

typedef int (*handleConnection)(void *pServerConnection, int cmd, void *param);

typedef struct MyServerConnection
{
    /*!
    For HTTP:
    Bits:0-16: accepted HTTP-verbs (GET,POST,...)
    */
    unsigned int options;
    MyServerConnectionInfo common;
    unsigned short port;
    /*@observer@*/ char *name;
    /*@null@*/ void *custom;
    SecureOptions secure;
    uint64_t normal_headers;     /*!< bitmask for normal headers*/
    uint64_t additional_headers; /*!< bitmask for security and client-hint headers*/
    MyServerClientConnections clients;
    handleConnection handleCommand;
    handleClientConnection handleClientCommand;
} MyServerConnection;

#endif