#ifndef SERVER_TYPES_INCLUDED
#define SERVER_TYPES_INCLUDED

#ifdef NUM_SOCKET_CONNECTIONS
#if NUM_SOCKET_CONNECTIONS < 1 || NUM_SOCKET_CONNECTIONS > 63
#error "NUM_SERVERS invalid (<1 or >63)."
#endif
#else
#define NUM_SOCKET_CONNECTIONS 2
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
    void *next;              /*!< next pointer in the linked list*/
    void *request;           /*!< request object */
    void *pServerConnection; /*!< pointer to server connection object */
    union
    {
#if USING_SSL
        SSL *ssl;
#endif
        void *reserved;
    } secure; /*!< used for security */
    union
    {
        char cbytes[CLIENT_CONNECTION_BUFFER_LEN];
    } buffer; /*!< some buffer */
} MyServerClientConnection;

typedef struct MyServerClientConnections
{
    size_t numCons;
    void *first;
    void *last;
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

typedef int (*handleConnection)(void *pConnection, int cmd, void *param);

typedef struct MyServerConnection
{
    /*!
    For HTTP:
    Bits:0-16: accepted HTTP-verbs (GET,POST,...)
    */
    unsigned int options;
    MyServerConnectionInfo common;
    unsigned short port;
    char *name;
    void *custom;
    SecureOptions secure;
    uint64_t normal_headers;     /*!< bitmask for normal headers*/
    uint64_t additional_headers; /*! bitmask for security and client-hint headers*/
    MyServerClientConnections clients;
    handleConnection handleCommand;
    handleClientConnection handleClientCommand;
} MyServerConnection;



typedef struct MyServer
{
    //void *next;
    MyServerConnection conns[NUM_SOCKET_CONNECTIONS];
    struct
    {
        unsigned long status;
        unsigned int options;
    } common;
    

} MyServer;

#endif