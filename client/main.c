#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "../SocketUtil/socketutil.h"
#include "../SocketUtil/tc.h"

void* ListenAndPrint(void *void_socketFD);
void StartListeningAndPrintMessagesOnSeperateThread(int* socketFD);

int main(int argc, char *argv[]){
    if (argc < 3){
        printf("please provide an ip and a port number\n");
        return 1;
    }
    char *ip = argv[1];
    int port = strtol(argv[2], NULL, 10);
    // ESTABLISHING CONNECTION
    int socketFD = CreateTCPIpv4Socket();

    struct sockaddr_in address;
    CreateTCPIpv4Address("127.0.0.1", port, &address);

    int result = connect(socketFD, (const struct sockaddr*) &address, sizeof(address));

    if(result == 0){
        printf("connection successful\n");
    }

    char buffer[1024];

    // SENDING AND RECIEVING DATA
    char *name = NULL;
    size_t name_size = 0;
    printf("What is your name?\n");
    ssize_t name_count = getline(&name, &name_size, stdin);
    name[name_count-1] = '\0';

    char *line = NULL;
    size_t line_size = 0;
    printf("type exit to exit ...\n");

    StartListeningAndPrintMessagesOnSeperateThread(&socketFD);


    while(true){
        line = readline("");
        sprintf(buffer, "%s: %s\n", name, line);

        if (strcmp(line, "exit") == 0) {
            sprintf(buffer, "%s left\n", name);
            ssize_t amount_sent = send(socketFD, buffer, strlen(buffer), 0);
            free(line);
            break;
        }

        ssize_t amount_sent = send(socketFD, buffer, strlen(buffer), 0);
        free(line);
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
            printf("%s%s%s", TC_GRN, buffer, TC_NRM);
        }
        if(amount_recieved == 0){
            break;
        }
    }
    close(socketFD);
    return 0;
}
