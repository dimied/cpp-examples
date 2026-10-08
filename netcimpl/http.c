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

static int running = 1;
static int retValue = 0;

// #define SERVER_NONBLOCKING 1

#define HEADER_DATA_SORTED_NAMES 0

#ifndef TESTING
#define TESTING 0
#endif

#define IP_EXAMPLE 0
#define SC_EXAMPLE 0
#define NET_EXAMPLE 0

#if IP_EXAMPLE
#include "ip_util.h"
void testParseIp()
#endif

#if SC_EXAMPLE
#include "linux_sysconf.h"

    void sysConfExample();
#endif

#if NET_EXAMPLE
struct addrinfo hints;

struct sockaddr_in socketAddr;

struct sockaddr_storage otherAddr;
socklen_t otherAddrLen;
int socketId;

int networkExample();
#endif

size_t writeFile(char *data, int len, int cnt)
{
    char buf[64];
    memset(buf, 0, 64);

    const int r = snprintf(buf, 64, "./request_%d.temp.txt", cnt);

    size_t res = 0;

    if (data != 0 && len > 0)
    {
        if (r < 64)
        {
            printf("W: %s\n", buf);
            FILE *fp = fopen(buf, "wb");
            if (((FILE *)0) != fp)
            {
                res = fwrite(data, 1, (size_t)len, fp);

                (void)fclose(fp);
            }
        }
    }
    return res;
}

#define INIT_HEADER_DATA_VALIDATE 1

#include "server.h"

static MyServerConnection myServerCon;

#define SIGHANDLER_MSG "Caught SIGINT (    )\n"

static char sighandler_msg_orig[] = SIGHANDLER_MSG;

static void signalHandler(int sig)
{
    char sighandler_msg[sizeof(SIGHANDLER_MSG)];
    memcpy(sighandler_msg, sighandler_msg_orig, sizeof(SIGHANDLER_MSG));

#define S2 (sizeof(SIGHANDLER_MSG))
    if (sig < 0)
    {
        sighandler_msg[S2 - 7] = '-';
        sig = -sig;
    }
    sighandler_msg[S2 - 4] = (char)((sig % 10) + (int)'0');
    sighandler_msg[S2 - 5] = (char)((sig / 10) % 10 + (int)'0');
    sighandler_msg[S2 - 6] = (char)((sig / 100) % 10 + (int)'0');
#undef S2

    int closeRes = cfc_close(&myServerCon);
    printf("Close res: %d\n", closeRes);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#if NET_EXAMPLE
    const char msg2[] = "Close socket\n";
    if (socketId != 0)
    {
        shutdown(socketId, SHUT_RDWR);
        close(socketId);

        write(STDOUT_FILENO, msg2, sizeof(msg2) - 1);
    }
#endif
    write(STDOUT_FILENO, sighandler_msg, sizeof(sighandler_msg) - 1);
#pragma GCC diagnostic pop
    running = 0;
    retValue = 1;
}

int yes = 1;
char buffer[1024];

// fd_set fdSet;

#include "http_parser_benchmark_snippets.h"
#include "http_parser_dev_helper.h"

#include "colors.h"

#define UINT64_TYPE(C) (C##ul)

#include "compression/bench_inflate.h"
#include "compression/gzip.h"
#include "compression/inflate_test.h"

int main()
{
    test_inflate();
    return 0;
    signal(SIGINT, signalHandler);

    // testParseIp();
    int cfr = cacheFiles();
    printf("Cache files: %d\n\n", cfr);

    printTypeSizes();

    int r = init_header_data();
    printf("INIT-HEADER-DATA: %d\n", r);
    if (r != 0)
    {
        return 2;
    }

    // printf("TODO: GUARD-BITS in header konstanten einbauen, sodass OR mit sec-header bits erkennbar wird.\n");

#if 0
    // bench_parse_method();
    // bench_parse_protocol();
    // bench_parse_text();
    bench_parse_headers();
    bench_parse_headers_parse_field();
    return 0;
#endif

#if TESTING
    testParser();
    // testHeaders();
    return 0;
#endif

    // printf("MASK: %ld\n", PARSE_IP_VALID_MASK);

    nullify(&myServerCon, sizeof(myServerCon));
    myServerCon.name = "localhost";
    myServerCon.port = 8080;
    myServerCon.options = (unsigned int)(SERVER_OPTION_TCP | SERVER_OPTION_IPV4);
    myServerCon.handleCommand = &basicHandle;
    myServerCon.handleClientCommand = &basicHandleClientConnection;

    sleep(1);

    int conRes = cfc_connect(&myServerCon);
    printf("CON-RES:%d\n", conRes);

    int listenRes2 = cfc_listen(&myServerCon);
    printf("LISTEN-RES:%d\n", listenRes2);
    // sleep(2);

    while (running != 0)
    {
        int acceptRes = cfc_accept(&myServerCon);
        printf("ACCEPT-RES: %d\n", acceptRes);
        sleep(1);
    }

    int closeRes = cfc_close(&myServerCon);
    printf("CLOSE-RES:%d\n", closeRes);

    return retValue;
}

#if SC_EXAMPLE
void sysConfExample()
{
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
}
#endif

#if NET_EXAMPLE

int networkExample()
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
#endif

#if IP_EXAMPLE
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

void testParseIp()
{
    for (size_t i = 0; 1 == 0 && i < NUM_TESTS; i++)
    {
        int parseRes = parse_ip(tests[i], 0);
        printf("IN: %s\nRES: %x (%s)(%s)\n", tests[i], parseRes, parse_ip_error(parseRes), parse_ip_success(parseRes));
    }
}
#endif