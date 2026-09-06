

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <signal.h>
// funcs: sleep
#include <unistd.h>

//
#include <sys/types.h>
#include <sys/socket.h>
// getprotobyname
#include <netdb.h>
#include <fcntl.h>

int epollId;

struct addrinfo hints;

struct sockaddr_in socketAddr;

struct sockaddr_storage otherAddr;
socklen_t otherAddrLen;
int socketId;
int running = 0, retValue = 0;
int yes = 1;

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

void signalHandler(int sig)
{
    (void)(sig);
    const char msg[] = "Caught SIGINT\n";
    const char msg2[] = "Close socket\n";

    if (socketId != 0)
    {
        shutdown(socketId, SHUT_RDWR);
        close(socketId);
        int res = write(STDOUT_FILENO, msg2, sizeof(msg2) - 1);
        (void)(res);
    }
    int res = write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    (void)(res);
    running = 0;
    retValue = 1;
    //_exit(sig);
}

void closeEpoll(int epollId)
{
    int res = close(epollId);
    printf("Close: %d\n", res);
}

int openSocket();

struct epoll_event ev;

int main()
{
    signal(SIGINT, signalHandler);

    printf("epoll...\n");
    epollId = epoll_create(16);
    printf("epoll-id: %d\n", epollId);
    if (epollId < 0)
    {
        printf("Failed!\n");
        return 1;
    }
    int res = openSocket();
    if (res != 0)
    {
        closeEpoll(epollId);
        printf("Failed to create socket\n");
    }

    printf("EV:size: %ld\n", sizeof(ev));

    memset(&ev, 0, sizeof(ev));

    ev.data.fd = socketId;
    ev.events = EPOLLIN;

    // max number of registered fds is in /proc/sys/fs/epoll/max_user_watches
    int ctlRes = epoll_ctl(epollId, EPOLL_CTL_ADD, socketId, &ev);
    printf("CTL-RES: %d\n", ctlRes);

    struct epoll_event events[1];
    running = 1;

    while (running != 0)
    {
        int waitRes = epoll_wait(epollId, events, 1, 0);
        printf("...: %d\n", waitRes);

        if (waitRes < 0)
        {
            break;
        }
        sleep(1);
    }
#if 0
    int fileId = open("./linux_epoll.c", O_RDONLY);
    printf("File: %d\n", ) if (fileId < 0)
    {
        closeEpoll(epollId);
        return 2;
    }
    close(fileId);
#endif
    //

    //
    close(socketId);
    closeEpoll(epollId);

    return retValue;
}

int openSocket()
{
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
    return 0;
}