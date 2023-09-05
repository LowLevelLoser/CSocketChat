#ifndef SOCKETUTIL_SOCKETUTIL_H
#define SOCKETUTIL_SOCKETUTIL_H

#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int CreateTCPIpv4Socket();
void CreateTCPIpv4Address(char *ip, int port, struct sockaddr_in *address);

#endif // !SOCKETUTIL_SOCKETUTIL_H
