#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8085
#define BUFFER_SIZE 1024

int main() {
    struct sockaddr_in serverAddress;
    int sock = 0;
    char* message = "Message from client";
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create error \n");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        printf("сonnect failed \n");
        exit(EXIT_FAILURE);
    } else {
        printf("сonnection succeed \n");
    }

    send(sock, message, strlen(message), 0);
    printf("Sent: %s\n", message);

    ssize_t bytesRead = recv(sock, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        printf("Response: %s\n", buffer);
    } else {
        perror("recv error");
    }

    close(sock);

    return 0;
}
