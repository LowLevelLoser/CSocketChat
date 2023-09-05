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
void RecieveAndPrintIncomingData(int socketFD);
void StartAcceptingIncomingConnections(int *server_socketFD);
void *AcceptIncomingConnectionWrapper(void *server_socketFD);
void *RecieveAndPrintIncomingDataOnSeperateThread(void *accepted_socket_ptr);

int main(){
 
    int server_socketFD = CreateTCPIpv4Socket();
    struct sockaddr_in server_address;
    CreateTCPIpv4Address("", 2000, &server_address);

    int bind_result = bind(server_socketFD, (const struct sockaddr*) &server_address, sizeof(server_address));
    if (bind_result == 0){
        printf("server socket was bound successfully\n");
    }

    int listen_result = listen(server_socketFD, 10);

    struct AcceptedSocket* client_socket = AcceptIncomingConnection(server_socketFD);
    StartAcceptingIncomingConnections(&server_socketFD);

    RecieveAndPrintIncomingData(client_socket->accepted_socketFD);

    shutdown(server_socketFD, SHUT_RDWR);

    return 0;
}

void StartAcceptingIncomingConnections(int* server_socketFD){
    pthread_t id;
    pthread_create(&id, NULL, AcceptIncomingConnectionWrapper, server_socketFD);

    //struct AcceptedSocket * AcceptIncomingConnection(server_socketFD);
}

void *AcceptConnectionAndRecieveData(void* server_socketFD){
    while(true){
        struct AcceptedSocket* client_socket = AcceptIncomingConnection(*((int *)server_socketFD));
        RecieveAndPrintIncomingData(client_socket->accepted_socketFD);
        close(client_socket->accepted_socketFD);
    }
    return 0;
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

void RecieveAndPrintIncomingData(int socketFD) {
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
}

void *RecieveAndPrintIncomingDataOnSeperateThread(void *accepted_socket_ptr){
    struct AcceptedSocket* client_socket = (struct AcceptedSocket*) accepted_socket_ptr;
    RecieveAndPrintIncomingData(client_socket->accepted_socketFD);
    close(client_socket->accepted_socketFD);
    return 0;
}

