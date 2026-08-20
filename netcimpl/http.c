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

void signalHandler(int sig)
{
    const char msg[] = "Caught SIGINT\n";
    const char msg2[] = "Close socket\n";

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
        printf("FAMILY: %d\n", pSocketAddr->sa_family);
    }
}

void printfAddrinfo(struct addrinfo *pInfo)
{
    printf("CNAME: %s\n", pInfo->ai_canonname);
    printf("FAMILY: %d\n", pInfo->ai_family);
    printf("SOCKTYPE: %d\n", pInfo->ai_socktype);
    printf("PROTOCOL: %d\n", pInfo->ai_protocol);
    printf("ADDR: %p\n", pInfo->ai_addr);
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

int main()
{
    signal(SIGINT, signalHandler);

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