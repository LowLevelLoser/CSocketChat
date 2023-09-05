#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include "../SocketUtil/socketutil.h"

int main(){

    // ESTABLISHING CONNECTION
    int socketFD = CreateTCPIpv4Socket();

    struct sockaddr_in address;
    CreateTCPIpv4Address("127.0.0.1", 2000, &address);

    int result = connect(socketFD, (const struct sockaddr*) &address, sizeof(address));

    if(result == 0){
        printf("connection successful\n");
    }

    // SENDING AND RECIEVING DATA

    char *line = NULL;
    size_t line_size = 0;
    printf("type exit to exit ...\n");

    while(true){
        ssize_t char_count = getline(&line, &line_size, stdin);
        if (char_count > 0){
            ssize_t amount_sent = send(socketFD, line, char_count, 0);
            if(strcmp(line, "exit\n") == 0){
                break;
            }
        }
    }

    close(socketFD);

    return 0;
}


