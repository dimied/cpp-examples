// func: printf,
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <signal.h>
// funcs: sleep
#include <unistd.h>

//
#include <sys/types.h>
#include <sys/socket.h>
// getprotobyname
#include <netdb.h>

int running = 1;
int retValue = 0;

struct addrinfo hints;

struct sockaddr_in socketAddr;

struct sockaddr_storage otherAddr;
socklen_t otherAddrLen;
int socketId;

// #define SERVER_NONBLOCKING 1

#include "server.h"

MyServer myServer;

void signalHandler(int sig)
{
    (void)(sig);
    const char msg[] = "Caught SIGINT\n";
    const char msg2[] = "Close socket\n";

    int closeRes = cfc_close(&myServer);
    printf("Close res: %d\n", closeRes);

    if (socketId != 0)
    {
        shutdown(socketId, SHUT_RDWR);
        close(socketId);
        write(STDOUT_FILENO, msg2, sizeof(msg2) - 1);
    }
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    running = 0;
    retValue = 1;
    //_exit(sig);
}

int yes = 1;
char buffer[1024];

// fd_set fdSet;

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

#include "linux_sysconf.h"

#include "ip_util.h"

const char *tests[] = {
    "127.0.0.1",
    "127.0.0+1",
    "127.0.0.G",
    "127.0.0.F",
    "::1",
    "1.1.1.1",
    "10.0.0.1",
    "10.255.0.1",
    "10.-255.0.1",
    "10.256.0.1",
    ":10.255.0.1",
    "::10.255.0.1",
    "1:2:3:4:5:6:7:8",
    "1:2:3:4::5.6.7.8"};
#define NUM_TESTS (sizeof(tests) / sizeof(tests[0]))

int main()
{
    signal(SIGINT, signalHandler);

    for (size_t i = 0; 1 == 0 && i < NUM_TESTS; i++)
    {
        int parseRes = parse_ip(tests[i], 0);
        printf("IN: %s\nRES: %x (%s)(%s)\n", tests[i], parseRes, parse_ip_error(parseRes), parse_ip_success(parseRes));
    }
    printf("MASK: %ld\n", PARSE_IP_VALID_MASK);

    nullify(&myServer, sizeof(myServer));
    myServer.conns[0].name = "localhost";
    myServer.conns[0].port = 8080;
    myServer.conns[0].options = SERVER_OPTION_TCP | SERVER_OPTION_IPV4;
    myServer.conns[0].handleCommand = basicHandle;
    myServer.conns[0].handleClientCommand = basicHandleClientConnection;

    int conRes = cfc_connect(&myServer);
    printf("CON-RES:%d\n", conRes);

    int listenRes2 = cfc_listen(&myServer);
    printf("LISTEN-RES:%d\n", listenRes2);
    sleep(3);
    int cfr = cacheFiles();
    printf("Cache files: %d\n", cfr);

    while (running != 0)
    {
        int acceptRes = cfc_accept(&myServer);
        printf("ACCEPT-RES: %d\n", acceptRes);
        sleep(1);
    }

    int closeRes = cfc_close(&myServer);
    printf("CLOSE-RES:%d\n", closeRes);

    return 0;

    char buf[200];
    memset(buf, 0, 200);

    unsigned int sysConfOptions = LINUX_SYS_CONF_DATA_USE_ALL;

    SysConfData *pSysConf = getSysConf(sysConfOptions, NULL);

    if (pSysConf != 0)
    {
        int pr = printSysData(pSysConf, sysConfOptions, buf, 200);
        freeSysConf(pSysConf);
        printf("----%d----\n", pr);
        printf("%s", buf);
        printf("---\n");
    }
    return 0;

    size_t size = 0;
    char *pContent = getContent(&size);
    if (pContent == 0)
    {
        printf("FAILED: Content!\n");
        return 1;
    }
    FILE *pFile = fopen("mimetypefast.h", "w");
    if (pFile == 0)
    {
        printf("FAILED: File!\n");
        printf("FREE:Content...\n");
        free(pContent);
        printf("FREE:Content...DONE\n");
        return 1;
    }
    size_t contentLen = strlen(pContent);
    printf("CL: %ld\n", contentLen);
    fwrite(pContent, 1, contentLen, pFile);

    int fileRes = fclose(pFile);
    printf("File close: %d\n", fileRes);
    printf("FREE:Content...\n");
    free(pContent);
    printf("FREE:Content...DONE\n");

    return 0;

    struct protoent *proto = getprotobyname("tcp");

    printf("PROTO: %d | %s\n", proto->p_proto, proto->p_name);
    printAliases(proto->p_aliases);
    // struct sockaddr *socketAddr = malloc(sizeof(sockaddr));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *myServerInfo;

    int res = getaddrinfo(NULL, "3344", &hints, &myServerInfo);

    printf("ADDR: %d\n", res);

    if (res != 0)
    {
        return 1;
        // printfAddrinfo(myServerInfo);
    }
    freeaddrinfo(myServerInfo);

    socketId = socket(AF_INET, SOCK_STREAM, 0);

    printf("SOCKET:RES: %d\n", socketId);
    if (socketId < 0)
    {
        printf("ERROR: socket!\n");
        return 1;
    }

    memset(&socketAddr, 0, sizeof(struct sockaddr_in));

    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = INADDR_ANY;
    socketAddr.sin_port = htons(8080);

    // to avoid already in use error
    res = setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (res != 0)
    {
        close(socketId);
        printf("Failed setsockopt...\n");
        return 1;
    }

    res = bind(socketId, (struct sockaddr *)&socketAddr, sizeof(struct sockaddr_in));

    printf("BIND: %d\n", res);

    if (res != 0)
    {
        close(socketId);
        return 1;
    }

    int backlogSize = 10;

    int listenRes = listen(socketId, backlogSize);

    printf("LISTEN: %d\n", listenRes);

    if (listenRes != 0)
    {
        close(socketId);
        return 1;
    }

    otherAddrLen = sizeof(otherAddr);
    printf("STORAGE: %ld\n", sizeof(otherAddr));

    printf("RUN...\n");

    while (running != 0)
    {
        printf("Waiting ... ");
        int acceptRes = accept(socketId, (struct sockaddr *)&otherAddr, &otherAddrLen);
        printf("Accept: %d\n", acceptRes);

        if (acceptRes != -1)
        {
            memset(buffer, 0, 1024);

            ssize_t recvRes = recv(acceptRes, buffer, 512, 0);
            printf("RECV: %ld\n", recvRes);
            printf("Content:\n%s\n", buffer);
            recvRes = send(acceptRes, "Hallo", 5, 0);
            printf("SENT: %ld\n", recvRes);
            sleep(5);
            printf("CLOSE CON!\n");
            close(acceptRes);
        }

        sleep(1);
    }
    close(socketId);

    return retValue;
}