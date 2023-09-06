#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include "../SocketUtil/socketutil.h"
#include <unistd.h>
#include <pthread.h>

struct AcceptedSocket{
    int accepted_socketFD;
    struct sockaddr_in address;
    int error;
    bool accepted_successfully;
};

struct AcceptedSocket * AcceptIncomingConnection(int server_socketFD);
void *RecieveAndPrintIncomingData(void* socketFD);
void StartAcceptingIncomingConnections(int *server_socketFD);
void RecieveAndPrintIncomingDataOnSeperateThread(int *p_socketFD);

int main(){
    int server_socketFD = CreateTCPIpv4Socket();
    struct sockaddr_in server_address;
    CreateTCPIpv4Address("", 2000, &server_address);

    int bind_result = bind(server_socketFD, (const struct sockaddr*) &server_address, sizeof(server_address));
    if (bind_result == 0){
        printf("server socket was bound successfully\n");
    }

    int listen_result = listen(server_socketFD, 10);

    StartAcceptingIncomingConnections(&server_socketFD);

    shutdown(server_socketFD, SHUT_RDWR);

    return 0;
}

void StartAcceptingIncomingConnections(int* server_socketFD){
    while(true){
        struct AcceptedSocket* client_socket = AcceptIncomingConnection(*server_socketFD);
        RecieveAndPrintIncomingDataOnSeperateThread(&client_socket->accepted_socketFD);
    }
}

struct AcceptedSocket * AcceptIncomingConnection(int server_socketFD){
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof (struct sockaddr_in);
    int client_socketFD = accept(server_socketFD, (struct sockaddr*) &client_address, &client_address_size);

    struct AcceptedSocket* accepted_socket = malloc(sizeof (struct AcceptedSocket));
    accepted_socket->address = client_address;
    accepted_socket->accepted_socketFD = client_socketFD;
    accepted_socket->accepted_successfully = client_socketFD > 0;

    if(accepted_socket->accepted_successfully == false){
        accepted_socket->error = client_socketFD;
    }
    return accepted_socket;
}

void RecieveAndPrintIncomingDataOnSeperateThread(int *p_socketFD){
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, RecieveAndPrintIncomingData, p_socketFD);
    printf("Someone joined\n");
}

void *RecieveAndPrintIncomingData(void* socketFD) {
    char buffer[1024];
    while (true) {
        ssize_t amount_recieved = recv(*((int *) socketFD), buffer, 1024, 0);
        if(amount_recieved > 0){
            buffer[amount_recieved] = '\0';
            printf("Response was: %s", buffer);
        }
        if(amount_recieved == 0){
            break;
        }
    }
    return 0;
}
