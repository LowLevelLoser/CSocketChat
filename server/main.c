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
#include <poll.h>


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

int main(int argc, char *argv[]){
    if (argc < 2){
        printf("please provide a port number\n");
        return 1;
    }
    int port = strtol(argv[1], NULL, 10);
    server_socketFD = CreateTCPIpv4Socket();
    struct sockaddr_in server_address;
    CreateTCPIpv4Address("", port, &server_address);

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
            if (pthread_join(threadIds[i], NULL) != 0){
                printf("failed to join thread");
            }
            else{
                printf("thread joined\n");
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
    // Allocate memory for the AcceptedSocket structure
    struct AcceptedSocket* accepted_socket = malloc(sizeof (struct AcceptedSocket));
    // Initialize accepted_successfully to false (default state)
    accepted_socket->accepted_successfully = false;

    // Set up a file descriptor array for poll
    struct pollfd fds[1];
    fds[0].fd = server_socketFD;  // Set the file descriptor to monitor
    fds[0].events = POLLIN;       // Monitor for readability

    // Use poll to wait for events on the file descriptor
    int result = poll(fds, 1, 2000); // Timeout is in milliseconds

    // Handle potential errors
    if (result == -1) {
        accepted_socket->error = -1;
        printf("Error in poll\n");
    }
    else if (result == 0) {
        accepted_socket->error = -2;
        printf("Timeout reached\n");
    }
    else if (fds[0].revents & POLLIN) {
        // If POLLIN event occurred, there's a connection waiting
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(struct sockaddr_in);

        // Accept the connection
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
            printf("connection established\n");
        }
        else {
            accepted_socket->error = client_socketFD; // Error in accept
        }
    }

    // Return the AcceptedSocket structure
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
