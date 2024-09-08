#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include "msg.h"

int setupSocket(const char *port) {
    struct addrinfo hints, *res, *rp;
    int serverFd, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;  
    hints.ai_flags = AI_PASSIVE;  
    hints.ai_protocol = 0;  

     // Retrieve list of address structs
    s = getaddrinfo(NULL, port, &hints, &res);
    if (s != 0) {
        printf("Error with getaddrinfo\n");
        return -1;
    }

    // Loop through all the results and bind
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        serverFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (serverFd == -1) continue;   // If socket fails, try next
        if (bind(serverFd, rp->ai_addr, rp->ai_addrlen) == 0) break;  // Successfully bound
        close(serverFd);
    }
    // Binding failed
    if (!rp) {
        fprintf(stderr, "Error with bind()\n");
        return -1;
    }
    // Listen on the bound socket
    if (listen(serverFd, SOMAXCONN) == -1) {
        perror("Error with listen()\n");
        close(serverFd);
        return -1;
    }

    freeaddrinfo(res);
    return serverFd;
}

void *clientHandler(void *socket) {
    int sock = *(int *)socket;
    free(socket);  // Clean up the allocated memory for the socket descriptor

    struct msg message;
    ssize_t readSize;
    int db_fd;  // File descriptor for the database file
    // Read messages from the client
    while ((readSize = read(sock, &message, sizeof(message))) > 0) {
        if (message.type == PUT) {
            // Handle PUT request
            db_fd = open("database.bin", O_WRONLY | O_CREAT | O_APPEND); // open file
            if (db_fd != -1) {
                // Write to database
                if (write(db_fd, &message.rd, sizeof(struct record)) == sizeof(struct record)) {
                    message.type = SUCCESS;
                } else {
                    message.type = FAIL;
                }
                close(db_fd);
            } else {
                message.type = FAIL;
            }
        } else if (message.type == GET) {
            // Handle GET request
            db_fd = open("database.bin", O_RDONLY); // open fle
            struct record tmp;
            int found = 0;

            if (db_fd != -1) {
                // Send data from database to client
                while (read(db_fd, &tmp, sizeof(struct record)) == sizeof(struct record)) {
                    if (tmp.id == message.rd.id) {
                        message.rd = tmp;
                        message.type = SUCCESS;
                        found = 1;
                        break;
                    }
                }
                close(db_fd);
                if (!found) message.type = FAIL;
            } else {
                message.type = FAIL;
            }
        }
        // Send the response back to the client
        write(sock, &message, sizeof(message));
    }

    if (readSize == -1) {
        perror("read failed");
    }

    close(sock);  // Close the client socket
    return NULL;
}

int main(int argc, char *argv[]) {
    // Check if correct number of args
    if (argc != 2) {
        printf("Invalid number of arguments\n");
        return EXIT_FAILURE;
    }

    int serverFd = setupSocket(argv[1]);
    if(serverFd == -1) return EXIT_FAILURE;

    struct sockaddr_storage clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    // Continuously accept new connections
    while (1) {
        // accept client
        int clientFd = accept(serverFd, (struct sockaddr *)&clientAddr, &clientAddrSize);
        if (clientFd < 0) {
            printf("Error with accept()\n");
            continue;
        }
        // Create thread to handle client
        pthread_t thread;
        int *newSocket = malloc(sizeof(int));
        *newSocket = clientFd;
        if (pthread_create(&thread, NULL, clientHandler, (void *)newSocket) != 0) {
            printf("Error creating thread\n");
            free(newSocket);
        }
    }
    // Close server socket
    close(serverFd);
    return 0;
}
