#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include "../SocketUtil/socketutil.h"

void* ListenAndPrint(void *void_socketFD);
void StartListeningAndPrintMessagesOnSeperateThread(int* socketFD);

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

    StartListeningAndPrintMessagesOnSeperateThread(&socketFD);

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

void StartListeningAndPrintMessagesOnSeperateThread(int* socketFD){
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, ListenAndPrint, (void *) socketFD);
}

void* ListenAndPrint(void *void_socketFD){

    int socketFD = *((int *) void_socketFD);
    char buffer[1024];
    while (true) {
        ssize_t amount_recieved = recv(socketFD, buffer, 1024, 0);
        if(amount_recieved > 0){
            buffer[amount_recieved] = '\0';
            printf("Response was: %s", buffer);
        }
        if(amount_recieved == 0){
            break;
        }
    }
    close(socketFD);
    return 0;
}
