#include "socketutil.h"
#include <netinet/in.h>
#include <string.h>

int CreateTCPIpv4Socket(){
    return socket(AF_INET, SOCK_STREAM, 0);
}

void CreateTCPIpv4Address(char *ip, int port, struct sockaddr_in *address){
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if(strlen(ip) == 0){
        address->sin_addr.s_addr = INADDR_ANY;
    }
    else{
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    }
}
