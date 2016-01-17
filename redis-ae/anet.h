#ifndef ANET_H
#define ANET_H

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

int anetRead(int fd, char *buf, int count);
int anetResolve(char *err, char *host, char *ipbuf);
int anetTcpServer(char *err, int port, char *bindaddr);
int anetTcpAccept(char *err, int serversock, char *ip, int *port);
int anetWrite(int fd, char *buf, int count);
int anetNonBlock(char *err, int fd);

#endif
