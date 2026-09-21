#ifndef MY_SERVER_INCLUDED
#define MY_SERVER_INCLUDED

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

#ifndef SSL_ENABLED
#define SSL_ENABLED 1
#endif

#ifdef SSL_ENABLED
#if SSL_ENABLED
#include <openssl/ssl.h>
#endif
#endif

#include <string.h>

#include "filecache.h"

#ifdef NUM_SOCKET_CONNECTIONS
#if NUM_SOCKET_CONNECTIONS < 1 || NUM_SOCKET_CONNECTIONS > 63
#error "NUM_SERVERS invalid (<1 or >63)."
#endif
#else
#define NUM_SOCKET_CONNECTIONS 2
#endif

#ifdef SERVER_OPTION_TCP
#error "SERVER_OPTION_TCP predefined."
#endif
#ifdef SERVER_OPTION_UDP
#error "SERVER_OPTION_UDP predefined."
#endif
#ifdef SERVER_OPTION_IPV4
#error "SERVER_OPTION_IPV4 predefined."
#endif
#ifdef SERVER_OPTION_IPV6
#error "SERVER_OPTION_IPV6 predefined."
#endif
#ifdef SERVER_OPTION_SECURE
#error "SERVER_OPTION_SECURE predefined."
#endif

#define SERVER_OPTION_TCP 0x10
#define SERVER_OPTION_UDP 0x20

#define SERVER_OPTION_IPV4 0x100
#define SERVER_OPTION_IPV6 0x200

#define SERVER_OPTION_SECURE 0x1000

#define SERVER_OPTION_COPY_NAME 0x10000

#define USING_SSL 1

#define CMD_ACCEPT 0x1
#define CMD_SKIP 0x2
#define CMD_CLOSE 0x3

#define STATE_UNKNOWN 0
#define STATE_ACCEPTED 0x100
#define STATE_CLOSED 0x200

#define STATE_RESERVED1 0x1000
#define STATE_HAS_ERRNO 0x2000
#define STATE_HAS_BUFFER 0x4000
#define SECURE_STATE 0x8000

#define IS_SECURE_CONNECTION(PTR_CC) ((PTR_CC->state & SECURE_STATE) != 0)

#define SET_PARSE_STATE(PTR_CC, STATE) (PTR_CC->state = (PTR_CC->state & 0xFFFF00) | STATE)
#define GET_PARSE_STATE(PTR_CC) (PTR_CC->state & 0xFF)
#define GET_PARSE_STATE_TO(PTR_CC, STATE) (STATE = GET_PARSE_STATE(PTR_CC))

#define SET_ERRNO(PTR_CC, ERRNO_VAL) (PTR_CC->state = (PTR_CC->state & 0xFF00FFFF) | STATE_HAS_ERRNO | (ERRNO_VAL << 16))
#define GET_ERRNO_FROM_STATE(STATE) ((STATE >> 16) & 0xFF)

#define CLIENT_CONNECTION_BUFFER_LEN 8

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

/*@null@*/ void *newClientConnection()
{
    return calloc(sizeof(MyServerClientConnection), 1);
}

typedef struct MyServerClientConnections
{
    size_t numCons;
    void *first;
    void *last;
} MyServerClientConnections;

typedef int (*handleClientConnection)(void *pClientConnection, void *pServerConnection);

typedef int (*handleConnection)(void *pConnection, int cmd, void *param);

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

#define SERVER_STATUS_ERROR_SSL_CERT_NOT_READABLE 0x100
#define SERVER_STATUS_ERROR_SSL_PRIV_KEY_NOT_READABLE 0x101

typedef struct MyServerConnection
{
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
    MyServerConnection conns[NUM_SOCKET_CONNECTIONS];
    struct
    {
        unsigned long status;
        unsigned int options;
    } common;
    void *next;

} MyServer;

#define HTTP_PARSER_DECLARATIONS 1
#define HTTP_PARSER_IMPLEMENTATION 1

void nullify(void *pData, size_t size)
{
    if (pData != 0 && size != 0)
    {
        memset(pData, 0, size);
    }
}

#include <openssl/err.h>

int errorCB(const char *str, size_t len, /*@temp@*/ void *_)
{
    (void)(_);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    (void)write(STDOUT_FILENO, str, len);
    (void)write(STDOUT_FILENO, "\n", 1);
#pragma GCC diagnostic pop
    // positive value to continue, 0 to stop
    return 1;
}

int handleSSLError(MyServerConnection *pConnection, int err)
{
    //(void)(pConnection);
    unsigned long lastError = ERR_get_error();
    printf("LAST-ERROR: %lu|%d|%p\n", lastError, err, (void *)pConnection);
    ERR_print_errors_cb(errorCB, /*@null@*/ 0);
    return err;
}

void printProto();

void printfAddrinfo(struct addrinfo *pInfo);

int addClientConnection(MyServerConnection *pCon, MyServerClientConnection *pCC);
void removeClientConnection(MyServerClientConnection *pCC);

int closeClientConnections(MyServerConnection *pCon)
{
    if (pCon != 0)
    {
        MyServerClientConnection *p = pCon->clients.first;

        while (p != 0)
        {
            MyServerClientConnection *next = p->next;
            if (p->state & SECURE_STATE)
            {
                SSL_shutdown(p->secure.ssl);
                SSL_free(p->secure.ssl);
            }
            if (p->socketId)
            {
                close(p->socketId);
            }
            free(p);
            p = next;
        }
        pCon->clients.first = 0;
        pCon->clients.last = 0;
        return 0;
    }
    return 1;
}

int cfc_close(MyServer *pServer)
{
    if (pServer == 0)
    {
        return 1;
    }

    MyServerConnection *pConnections = pServer->conns;

    int conIdx;
    for (conIdx = 0; conIdx < NUM_SOCKET_CONNECTIONS; conIdx++)
    {
        if (pConnections->common.socketId)
        {
            closeClientConnections(pConnections);
            if (pConnections->secure.ctx)
            {
                SSL_CTX_free(pConnections->secure.ctx);
                pConnections->secure.ctx = 0;
            }
            int res = close(pConnections->common.socketId);
            printf("CLOSE:%d;socket:%d;res:%d;\n", conIdx, pConnections->common.socketId, res);
            pConnections->common.socketId = 0;
        }
        ++pConnections;
    }
    return 0;
}

// TODO: implement SSL_CTX_config
#if 0
if (CONF_modules_load_file("config.cnf", "testapp", 0) <= 0) {
    fprintf(stderr, "Error processing config file\n");
    goto err;
}

ctx = SSL_CTX_new(TLS_server_method());

if (SSL_CTX_config(ctx, "server") == 0) {
    fprintf(stderr, "Error configuring server.\n");
    goto err;
}
#endif

// place in read-only memory
const int yesOption = 1;

int prepareSSL(MyServerConnection *pConnection)
{
    if (pConnection != 0)
    {
        if (pConnection->options & SERVER_OPTION_SECURE)
        {
            // test if files are accessible
            int ssl_error = 0;
            int fr = access(pConnection->secure.certFilePath, F_OK | R_OK);
            if (fr != 0)
            {
                return handleSSLError(pConnection, 100);
            }
            fr = access(pConnection->secure.privKeyFilePath, F_OK | R_OK);
            if (fr != 0)
            {
                return handleSSLError(pConnection, 200);
            }

            SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());

            if (0 == ctx)
            {
                return handleSSLError(pConnection, 1);
            }
            else
            {
                int r = SSL_CTX_use_certificate_chain_file(ctx, pConnection->secure.certFilePath);
                if (1 == r)
                {
                    r = SSL_CTX_use_PrivateKey_file(ctx, pConnection->secure.privKeyFilePath, SSL_FILETYPE_PEM);
                    if (1 != r)
                    {
                        ssl_error = handleSSLError(pConnection, 3);
                        SSL_CTX_free(ctx);
                    }
                }
                else
                {
                    ssl_error = handleSSLError(pConnection, 2);
                    SSL_CTX_free(ctx);
                }
            }

            if (0 != ssl_error)
            {
                pConnection->common.status = ssl_error;
                return ssl_error;
            }
            else
            {
                pConnection->secure.ctx = ctx;
            }
        }
        return 0;
    }
    return 1;
}

/*
 */
int cfc_connect(MyServer *pServer)
{
    if (pServer == 0)
    {
        return 1;
    }

    MyServerConnection *pConnection = pServer->conns;
#if 0
    struct addrinfo hints;

    union
    {
        unsigned long value;
        char data[8];
    } portNumber;
#endif

    union
    {
        struct sockaddr_in addrV4;
        struct sockaddr_in6 addrV6;
    } mySockaddr;

    unsigned long status = 0;

    for (size_t conIdx = 0; conIdx < NUM_SOCKET_CONNECTIONS; conIdx++)
    {
#if 0
        nullify(&hints, sizeof(hints));
#endif
        int ai_family = AF_UNSPEC, ai_socktype;

        if (0 != prepareSSL(pConnection))
        {
            continue;
        }

        if (pConnection->options & SERVER_OPTION_IPV4)
        {
            ai_family = AF_INET;
        }
        else if (pConnection->options & SERVER_OPTION_IPV6)
        {
            ai_family = AF_INET6;
        }

        int assigned = 0;

        if (pConnection->options & SERVER_OPTION_TCP)
        {
            ++assigned;
            ai_socktype = SOCK_STREAM;
#if 0
            hints.ai_flags = AI_PASSIVE;
#endif
            // printf("Connect TCP\n");
        }
        else if (pConnection->options & SERVER_OPTION_UDP)
        {
            ++assigned;
            ai_socktype = SOCK_DGRAM;
#if 0
            hints.ai_flags = AI_PASSIVE;
#endif
            // printf("Connect UDP\n");
        }
        else
        {
            continue;
        }

        if (assigned)
        {
            assigned = 0;
            if (ai_family == AF_UNSPEC)
            {
                ai_family = AF_INET;
            }
#if 0
            hints.ai_family = ai_family;
            hints.ai_socktype = ai_socktype;

            struct addrinfo *myServerInfo = 0;

            nullify(portNumber.data, 8);
            int j = 0;
#if 1
            {
                int port = pConnection->port;
                if (0 == port)
                {
                    portNumber.data[0] = '0';
                }
                else
                {
                    j = 8;
                    while (port != 0)
                    {
                        --j;
                        portNumber.data[j] = (port % 10) + '0';
                        port /= 10;
                    }
                }
            }
#else
            sprintf(portNumber, "%d", pConnections->port);
#endif

            printf("PORT?%s?\n", &portNumber.data[j]);
            int resAddr = getaddrinfo(pConnection->name, 0, &hints, &myServerInfo);

            printf("ADDR: %d|%s|%s|%d\n", res, pConnection->name, &portNumber.data[j], pConnection->port);

            if (res == 0)
            {
                printf("--ADDR-INFO\n");
                printfAddrinfo(myServerInfo);
                printf("--ADDR-INFO-END\n");
            }
            else
            {
                char *errStr = gai_strerror(res);
                printf("ERR: %s\n", errStr);
            }

            if (myServerInfo != 0)
            {
                freeaddrinfo(myServerInfo);
            }
#endif
            int socketId = socket(ai_family, ai_socktype, 0);

            pConnection->common.socketId = socketId;

            if (socketId < 0)
            {
                printf("Error: socket!\n");
                ++assigned;
                // TODO:errors
            }
            else
            {
                // to avoid already in use error
                int res = setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, &yesOption, sizeof(yesOption));
                if (res != 0)
                {
                    close(socketId);
                    pConnection->common.socketId = 0;
                    printf("Failed setsockopt...\n");
                    ++assigned;
                }

#ifdef SERVER_NONBLOCKING
#if SERVER_NONBLOCKING
                // Non-blocking
                int flags = fcntl(socketId, F_GETFL, 0);
                fcntl(socketId, F_SETFL, flags | O_NONBLOCK);
#endif
#endif

                nullify(&mySockaddr, sizeof(mySockaddr));

                socklen_t sockLen = sizeof(mySockaddr.addrV4);
                in_port_t bindPort = htons(pConnection->port);

                if (ai_family == AF_INET)
                {
                    mySockaddr.addrV4.sin_family = AF_INET;
                    mySockaddr.addrV4.sin_addr.s_addr = INADDR_ANY;
                    mySockaddr.addrV4.sin_port = bindPort;
                }
                else
                {
                    mySockaddr.addrV6.sin6_family = AF_INET6;
                    // mySockaddr.addrV6.sin6_addr = IN6ADDR_ANY_INIT;
                    memcpy(&mySockaddr.addrV6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
                    mySockaddr.addrV6.sin6_port = bindPort;
                    sockLen = sizeof(mySockaddr.addrV6);
                }

                res = bind(socketId, (struct sockaddr *)&mySockaddr, sockLen);

                if (res != 0)
                {
                    close(socketId);
                    pConnection->common.socketId = 0;
                    printf("Failed bind...\n");
                    ++assigned;
                }
            }
        }

        if (0 == assigned)
        {
            status |= (0x1ul) << conIdx;
        }

        ++pConnection;
    }

    pServer->common.status = status;

    return 0;
}

int cfc_listen(MyServer *pServer)
{
    if (0 == pServer)
    {
        return 1;
    }

    unsigned long status = 0;

    int backlogSize = 16;
    MyServerConnection *pConnections = pServer->conns;

    for (size_t conIdx = 0; conIdx < NUM_SOCKET_CONNECTIONS; conIdx++)
    {
        const int socketId = pConnections->common.socketId;

        if (socketId > 0)
        {
            int listenRes = listen(socketId, backlogSize);

            printf("LISTEN: %d\n", listenRes);

            if (listenRes == -1)
            {
                printf("ERRNO: %s\n", strerror(errno));
                close(socketId);
                pConnections->common.socketId = 0;
            }
            else
            {
                status |= (0x1ul) << conIdx;
            }
        }
        ++pConnections;
    }

    pServer->common.status = status;

    return 0;
}

int cfc_accept(MyServer *pServer)
{
    if (0 == pServer)
    {
        return 1;
    }

    MyServerConnection *pConnections = pServer->conns;

    printf("RUN...\n");

    for (size_t conIdx = 0; conIdx < NUM_SOCKET_CONNECTIONS; conIdx++)
    {
        printf("ACCEPT?: %d\n", pConnections->common.socketId);

        if (pConnections->common.socketId)
        {
            if (pConnections->handleCommand)
            {
                int r = pConnections->handleCommand(pConnections, CMD_ACCEPT, pServer);
                printf("H: %d\n", r);
            }
        }
        ++pConnections;
    }

    return 0;
}

int basicHandle(void *pConnection, int cmd, void *param)
{
    (void)(param);

    if (pConnection != 0)
    {
        MyServerConnection *pCon = (MyServerConnection *)pConnection;
        struct sockaddr_storage otherAddr;
        socklen_t otherAddrLen;

        otherAddrLen = sizeof(otherAddr);

        if (pCon->common.socketId)
        {
            handleClientConnection pCCfunc = pCon->handleClientCommand;

            if (0 == pCCfunc)
            {
                printf("NO Client cmd!\n");
                return 3;
            }

            if (CMD_ACCEPT == cmd)
            {
                printf("Waiting... %d\n", pCon->common.socketId);

                int clientFd = accept(pCon->common.socketId, (struct sockaddr *)&otherAddr, &otherAddrLen);

                if (clientFd != -1)
                {
                    printf("Accepted: %d\n", clientFd);
                    printf("STORAGE: %ld\n", sizeof(otherAddr));

                    MyServerClientConnection *pCC = newClientConnection();

                    if (pCC == 0)
                    {
                        return 3;
                    }

                    if (pCon->options & SERVER_OPTION_SECURE)
                    {
                        SSL_CTX *ctx = pCon->secure.ctx;

                        if (ctx != 0)
                        {
                            SSL *ssl = SSL_new(ctx);

                            if (ssl != 0)
                            {
                                int r = SSL_set_fd(ssl, clientFd);
                                printf("SET: %d\n", r);

                                r = SSL_accept(ssl);
                                printf("ACC: %d\n", r);

                                pCC->secure.ssl = ssl;
                                pCC->state |= SECURE_STATE;
                            }
                            else
                            {
                                // TODO: Allow empty connections?
                                return 300;
                            }
                        }
                        else
                        {
                            // Impossible
                            return 300;
                        }
                    }
                    pCC->socketId = clientFd;
                    pCC->state |= STATE_ACCEPTED;

                    int r = addClientConnection(pCon, pCC);

                    if (r != 0)
                    {
                        if (pCC->state & SECURE_STATE)
                        {
                            SSL_shutdown(pCC->secure.ssl);
                            SSL_free(pCC->secure.ssl);
                            pCC->secure.ssl = 0;
                        }
                        free(pCC);

                        return 100;
                    }

                    return pCCfunc(pCC, pCon);
                }
                else
                {
                    if (errno == EAGAIN)
                    {
                        return 4;
                    }
                    printf("ERR: %d | %s\n", errno, strerror(errno));
                    return 2;
                }
            }
        }
    }
    return 1;
}

void freeRequest(void *p);

int verbose = 1;

void closeCC(MyServerClientConnection *pCC)
{
    printf("CLOSE CON!\n");
#if TESTING == 0
    if (pCC->socketId != 0)
    {
        printf("Close socket!\n");
        close(pCC->socketId);
        pCC->socketId = 0;
    }
#endif
    if (pCC->request != 0)
    {
        freeRequest(pCC->request);
        pCC->request = 0;
    }

    pCC->state = STATE_CLOSED | (pCC->state & SECURE_STATE);
}

char *badRequest = "HTTP/1.1 400 Bad Request";

void closeBadRequest(MyServerClientConnection *pCC)
{
    printf("CLOSE BAD!\n");
#if TESTING == 0
    if (pCC->socketId)
    {
        // TODO: error handling
        send(pCC->socketId, badRequest, strlen(badRequest), 0);
    }
#endif
    closeCC(pCC);
}

int addClientConnection(MyServerConnection *pCon, MyServerClientConnection *pCC)
{
    pCC->pServerConnection = pCon;
    MyServerClientConnection *pLast = (MyServerClientConnection *)pCon->clients.last;
    if (pLast != 0)
    {
        printf("Append con\n");
        pLast->next = pCC;
    }
    pLast = pCC;
    pCon->clients.last = pLast;
    if (0 == pCon->clients.first)
    {
        pCon->clients.first = pLast;
    }
    ++pCon->clients.numCons;
    return 0;
}

void removeClientConnection(MyServerClientConnection *pCC)
{
    if (pCC != 0)
    {
        MyServerConnection *pCon = (MyServerConnection *)pCC->pServerConnection;
        if (pCon != 0)
        {
            MyServerClientConnection *pLast = pCon->clients.first;
            if (pLast == pCC)
            {
                MyServerClientConnection *next = pCC->next;
                closeCC(pCC);
                pCon->clients.first = next;

                if (pCC == pCon->clients.last)
                {
                    pCon->clients.last = 0;
                }
                return;
            }

            MyServerClientConnection *next;
            MyServerClientConnection *p = pLast->next;

            while (p != 0)
            {
                next = p->next;
                if (p == pCC)
                {
                    pLast->next = next;
                    break;
                }
                pLast = p;
                p = next;
            }
            if (pCon->clients.last == pCC)
            {
                pCon->clients.last = pLast;
            }
        }
    }
}

#define IS_VALID_HOST_OK 0
#define IS_VALID_HOST_ERROR_INVALID_ARG 1
#define IS_VALID_HOST_ERROR_SERVER_CON 2
#define IS_VALID_HOST_ERROR_INVALID_HOST 3
#define IS_VALID_HOST_ERROR_PARSE_PORT 4
#define IS_VALID_HOST_ERROR_INVALID_PORT 5
#define IS_VALID_HOST_ERROR_REQUIRES_PORT 6

int isValidHost(MyServerClientConnection *pCC, char *hostname)
{
    if (hostname != 0 && pCC != 0)
    {
        MyServerConnection *pSC = (MyServerConnection *)pCC->pServerConnection;
        if (pSC == 0)
        {
            return IS_VALID_HOST_ERROR_SERVER_CON;
        }

        while (*hostname != 0 && *hostname == ' ')
        {
            ++hostname;
        }

        if (*hostname == 0)
        {
            return IS_VALID_HOST_ERROR_INVALID_HOST;
        }

        const char *domain = hostname;

        while (*hostname != 0 && *hostname != ':')
        {
            ++hostname;
        }

        if (hostname > domain)
        {
            const size_t l = (hostname - domain);
            const size_t l2 = strlen(pSC->name);

            if (l == l2 && strncmp(pSC->name, domain, l) == 0)
            {
                const int expectedPort = (int)pSC->port;
                int host_port = -1;

                if (*hostname != 0)
                {
                    ++hostname;
                    ++host_port;

#if 0
                    uint64_t v = (uint64_t)hostname;
                    printf("Off: %ld\n", v & 0x7);
                    if ((v & 0x7) == 0)
                    {
                        printf("Aligned!\n");
                    }
#endif
                    while (*hostname != 0 && host_port < 0x10000)
                    {
                        if (*hostname < '0' || *hostname > '9')
                        {
                            return IS_VALID_HOST_ERROR_PARSE_PORT;
                        }
                        host_port = host_port * 10 + (*hostname - '0');
                        ++hostname;
                    }

                    if (host_port != expectedPort)
                    {
                        return IS_VALID_HOST_ERROR_INVALID_PORT;
                    }
                    return IS_VALID_HOST_OK;
                }

                if (expectedPort == 80 || expectedPort == 443)
                {
                    return IS_VALID_HOST_OK;
                }
                // Non-standard port, i.e. port is required in "host header value"
                return IS_VALID_HOST_ERROR_REQUIRES_PORT;
            }
        }
    }
    return IS_VALID_HOST_ERROR_INVALID_ARG;
}

#include "http_parser.h"

void printAliases(char **aliases)
{
    printf("PROTO:ALIASES:");

    int num = 0;
    while (*aliases != 0)
    {
        if (num > 0)
        {
            printf(",");
        }
        printf("%s", *aliases);
        num++;
        aliases++;
    }

    printf("\n");
}

void printSocketAddr(struct sockaddr *pSocketAddr, socklen_t addrLen)
{
    if (pSocketAddr != 0)
    {
        printf("ADDR:size: %d\n", addrLen);
        printf("FAMILY: %d\n", pSocketAddr->sa_family);
    }
}

void printfAddrinfo(struct addrinfo *pInfo)
{
    printf("CNAME: %s\n", pInfo->ai_canonname);
    printf("FAMILY: %d\n", pInfo->ai_family);
    printf("SOCKTYPE: %d\n", pInfo->ai_socktype);
    printf("PROTOCOL: %d\n", pInfo->ai_protocol);
    printf("ADDR: %p\n", (void *)pInfo->ai_addr);
    printSocketAddr(pInfo->ai_addr, pInfo->ai_addrlen);

    /*
    struct addrinfo
{
  int ai_flags;			// Input flags.
  int ai_family;		// Protocol family for socket.
  int ai_socktype;		// Socket type.
  int ai_protocol;		// Protocol for socket.
  socklen_t ai_addrlen;		// Length of socket address.
  struct sockaddr *ai_addr;	// Socket address for socket.
  char *ai_canonname;		// Canonical name for service location.
  struct addrinfo *ai_next;	// Pointer to next in list.
};
*/
    if (pInfo->ai_next != 0)
    {
        printfAddrinfo(pInfo->ai_next);
    }
}

void printProto()
{
    struct protoent *proto = getprotobyname("tcp");

    printf("PROTO: %d | %s\n", proto->p_proto, proto->p_name);
    printAliases(proto->p_aliases);
}

void printTypeSizes()
{
    printf("Server:size:%ld\n", sizeof(MyServer));
    printf("Ser-Con:size:%ld\n", sizeof(MyServerConnection));
    printf("Cli-Con:size:%ld\n", sizeof(MyServerClientConnection));
    printf("Request:size:%ld\n", sizeof(HttpRequest));
    printf("Headers:size:%ld\n", sizeof(HeadersDataHeader));
    printf("\n");
}
#endif