#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define PORT 8085
#define LIMIT 5
#define BUFFER_SIZE 1024

volatile sig_atomic_t sighupReceived = 0;

void sigHupHandler(int sigNumber) {
    sighupReceived = 1;
    printf("SigHub\n");
}

void handleConnection(int* incomingSocketFD) {
    char buffer[BUFFER_SIZE] = { 0 };
    int readBytes = read(*incomingSocketFD, buffer, BUFFER_SIZE);

    if (readBytes > 0) { 
        printf("Received data: %d bytes\n", readBytes);
        printf("Received message from client: %s\n", buffer);

        const char* response = "Hello from server!";
        if (send(*incomingSocketFD, response, strlen(response), 0) < 0) {
            perror("send error");
        }

    } else {
        if (readBytes == 0) {
            close(*incomingSocketFD); 
            *incomingSocketFD = 0; 
            printf("Connection closed\n\n");
        } else { 
            perror("read error"); 
        }  
    } 
}

int main() {
    int serverFD;
    int incomingSocketFD = 0; 
    struct sockaddr_in socketAddress; 
    int addressLength = sizeof(socketAddress);
    fd_set readfds;
    sigset_t blockedMask, origMask;
    int maxSd;

    if ((serverFD = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = INADDR_ANY;
    socketAddress.sin_port = htons(PORT);

    if (bind(serverFD, (struct sockaddr*)&socketAddress, sizeof(socketAddress)) < 0) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFD, LIMIT) < 0) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    printf("Server started, port %d \n\n", PORT);

    struct sigaction sa;
    int sigactionFirst = sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    int sigactionSecond = sigaction(SIGHUP, &sa, NULL);

    sigemptyset(&blockedMask);
    sigemptyset(&origMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    while (1) {

        FD_ZERO(&readfds); 
        FD_SET(serverFD, &readfds); 
        
        if (incomingSocketFD > 0) { 
            FD_SET(incomingSocketFD, &readfds); 
        } 
        
        if (incomingSocketFD > serverFD){
            maxSd =  incomingSocketFD;
        } else {
            maxSd = serverFD; 
        } 
 
        if (pselect(maxSd + 1, &readfds, NULL, NULL, NULL, &origMask) != -1)
        {
            if (sighupReceived) {
                printf("SIGHUP received.\n");
                sighupReceived = 0;
                continue;
            }

            
        } else {
            if (errno != EINTR) {
                perror("pselect error"); 
                exit(EXIT_FAILURE); 
            }
        }

        if (incomingSocketFD > 0 && FD_ISSET(incomingSocketFD, &readfds)) { 
            handleConnection(&incomingSocketFD);
            continue;
        }
        
        if (FD_ISSET(serverFD, &readfds)) {
            if ((incomingSocketFD = accept(serverFD, (struct sockaddr*)&socketAddress, (socklen_t*)&addressLength)) < 0) {
                perror("accept error");
                exit(EXIT_FAILURE);
            }

            printf("New connection.\n");


        }

        
    }

    close(serverFD);

    return 0;
}
