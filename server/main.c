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
#include <sys/time.h>

#define NUM_OF_CLIENTS 10
#define NOT_INITIALIZED 0
#define ACTIVE 1
#define JOIN 2

struct AcceptedSocket{
    int accepted_socketFD;
    struct sockaddr_in address;
    int error;
    bool accepted_successfully;
};
struct AcceptedSocket acceptedSockets[NUM_OF_CLIENTS];
int threadIndex = 0;
pthread_t threadIds[10];
int threadCondition[10] = {NOT_INITIALIZED};
int hackyEndProgramVariable = 0;
bool hasClientJoined = false;
bool createNewThread = false;
int acceptedSocketsCount = 0;
int acceptedSocketsIndex = 0;
int server_socketFD;

struct AcceptedSocket * AcceptIncomingConnection(int server_socketFD);
void *RecieveAndPrintIncomingData(void* socketFD);
void StartAcceptingIncomingConnections(int server_socketFD);
void RecieveAndPrintIncomingDataOnSeperateThread(int *p_socketFD);
void SendMessageToOtherClients(char* buffer, int sockFD);
void CleanUp();

bool exitDebug = true;

int main(){
    server_socketFD = CreateTCPIpv4Socket();
    struct sockaddr_in server_address;
    CreateTCPIpv4Address("", 2000, &server_address);

    int bind_result = bind(server_socketFD, (const struct sockaddr*) &server_address, sizeof(server_address));
    if (bind_result == 0){
        printf("server socket was bound successfully\n");
    }

    int listen_result = listen(server_socketFD, NUM_OF_CLIENTS);

    StartAcceptingIncomingConnections(server_socketFD);
    for(int i = 0; i < threadIndex; i++){
        printf("thread id on clean %lu\n",threadIds[i]);
        printf("thread condition %d\n",threadCondition[i]);
        if(threadCondition[i] == JOIN){
            printf("thread joined\n");
            if (pthread_join(threadIds[i], NULL) != 0){
                printf("failed to join thread");
            }
            else{
                printf("thread terminated\n");
            }
        }
    }
    shutdown(server_socketFD, SHUT_RDWR);
    printf("everyone left\n");

    return 0;
}

void StartAcceptingIncomingConnections(int server_socketFD){
    while(true){
        struct AcceptedSocket* client_socket = AcceptIncomingConnection(server_socketFD);
        if(!(acceptedSocketsCount > 0 || hasClientJoined == false)){
            break;
        }
        if(createNewThread){
            RecieveAndPrintIncomingDataOnSeperateThread(&client_socket->accepted_socketFD);
        }
        createNewThread = false;
    }
    return;
}

struct AcceptedSocket * AcceptIncomingConnection(int server_socketFD){
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(server_socketFD, &readfds);

    int result = select(server_socketFD + 1, &readfds, NULL, NULL, &timeout);

    struct AcceptedSocket* accepted_socket = malloc(sizeof (struct AcceptedSocket));
    accepted_socket->accepted_successfully = false;

    if (result == -1) {
        accepted_socket->error = -1; // Error in select
    }
    else if (result == 0) {
        accepted_socket->error = -2; // Timeout reached
        printf("timeout\n");
    }
    else {
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(struct sockaddr_in);
        int client_socketFD = accept(server_socketFD, (struct sockaddr*) &client_address, &client_address_size);

        if (client_socketFD > 0) {
            accepted_socket->accepted_socketFD = client_socketFD;
            accepted_socket->address = client_address;
            accepted_socket->accepted_successfully = true;
            hasClientJoined = true;
            acceptedSocketsCount++;
            acceptedSockets[acceptedSocketsIndex] = *accepted_socket;
            acceptedSocketsIndex++;
            createNewThread = true;
        }
        else {
            accepted_socket->error = client_socketFD; // Error in accept
        }
    }

    return accepted_socket;
}
//select
void RecieveAndPrintIncomingDataOnSeperateThread(int *p_socketFD){
    threadCondition[threadIndex] = ACTIVE;
    pthread_create(&threadIds[threadIndex], NULL, RecieveAndPrintIncomingData, p_socketFD);
    threadIndex++;
    SendMessageToOtherClients("Someone joined\n", *p_socketFD);
}

void *RecieveAndPrintIncomingData(void* socketFD) {
    pthread_t thread_id = pthread_self();
    printf("Thread ID from thread: %lu\n", thread_id);
    char buffer[1024];
    while (true) {
        ssize_t amount_recieved = recv(*((int *) socketFD), buffer, 1024, 0);
        if(amount_recieved > 0){
            buffer[amount_recieved] = '\0';
            printf("%s", buffer);
            SendMessageToOtherClients(buffer, *((int *) socketFD));
        }
        if(amount_recieved == 0){
            acceptedSocketsCount--;
            break;
        }
    }

    for (int i = 0; i < NUM_OF_CLIENTS; i++) {
        if (thread_id == threadIds[i]){
            threadCondition[i] = JOIN;
            printf("flagged thread for termination\n");
            break;
        }
    }
    exitDebug = false;
    pthread_exit(NULL);
    return 0;
}

void SendMessageToOtherClients(char* buffer, int sockFD){
    for (int i = 0; i < acceptedSocketsIndex; i++){
        if(acceptedSockets[i].accepted_socketFD != sockFD){
            send(acceptedSockets[i].accepted_socketFD, buffer, strlen(buffer), 0);
        }
    }
}
