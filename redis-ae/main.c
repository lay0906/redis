#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "ae.h"
#include "anet.h"

#define PORT 1234
#define MAX_LEN 1024

char g_err_string[1024];

aeEventLoop *g_event_loop = NULL;

void stopEcho()
{
    aeStop(g_event_loop);
	printf("the echo server is closed by sigint!");
}

void closeConn(aeEventLoop *loop, int fd, int err)
{
    if(0 == err)
        printf("client[fd:%d] quit\n", fd);
    else if(-1 == err)
        fprintf(stderr, "close client[fd:%d] error: %s\n", fd, strerror(errno));
    aeDeleteFileEvent(loop, fd, AE_READABLE);
    close(fd);
}

void readData(aeEventLoop *loop, int fd, void *data, int mask)
{
    char buffer[MAX_LEN] = {0};
    int n;
    n = read(fd, buffer, MAX_LEN);
    if(n <= 0)
    {
        closeConn(loop, fd, n);
    }
    else
    {
        n = write(fd, buffer, MAX_LEN);
        if(-1 == n)
            closeConn(loop, fd, n);
    }
}

void acceptConn(aeEventLoop *loop, int fd, void *data, int mask)
{
    int connfd, connport;
    char ip_addr[128] = {0};
    connfd = anetTcpAccept(g_err_string, fd, ip_addr, &connport);
    printf("connected from %s:%d\n", ip_addr, connport);

    if(aeCreateFileEvent(loop, connfd, AE_READABLE,readData, NULL) == AE_ERR)
    {
        fprintf(stderr, "client[fd:%d] connect fail\n", fd);
        close(fd);
    }
}

int main()
{

    printf("start echo server...\n");

    signal(SIGINT, stopEcho);

    g_event_loop = aeCreateEventLoop(1024*10);

    int fd = anetTcpServer(g_err_string, PORT, NULL);
    if(ANET_ERR == fd)
        fprintf(stderr, "start server in port[%d] error: %s\n", PORT, g_err_string);
    if(aeCreateFileEvent(g_event_loop, fd, AE_READABLE, acceptConn, NULL) == AE_ERR )
        fprintf(stderr, "some uncaughtable error happend!");

	printf("start success!\n");
	aeMain(g_event_loop);

    aeDeleteEventLoop(g_event_loop);

    printf("stop success!\n");

    return 0;
}
