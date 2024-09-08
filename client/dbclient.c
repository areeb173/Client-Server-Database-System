#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  
#include <sys/socket.h>      
#include <netdb.h>           
#include <arpa/inet.h>       
#include "msg.h"       

#define BUFFSIZE 256    // buffsize for message

// Function for connecting to server
int connectToServer(const char *serverIP, const char *port) {
    struct addrinfo hints, *res;
    int sock = -1;
    // initialize hints struct
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = 0;
    hints.ai_protocol = 0; 
    // Get address structures for the specified server IP and port
    int s = getaddrinfo(serverIP, port, &hints, &res);
    if (s != 0) {
        printf("Error with getaddrinfo()\n");
        return -1;
    }
    // If result in not NULL
    if(res) {
        // Create a socket
        sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(!(sock < 0)){
            // Try to establish a connection on the socket
            if(connect(sock, res->ai_addr, res->ai_addrlen) != 0){
                printf("Connection Failed");
                close(sock);
                return -1;
            }
        } 
        else {
            perror("Cannot create socket");
            return -1;
        }
    }

    freeaddrinfo(res);
    return sock;
}

int main(int argc, char **argv) {

    int sock = connectToServer(argv[1], argv[2]);
    if (sock < 0) {
        printf("Unable to connect to server\n");
        return -1;
    }

    struct msg message;
    int choice;

    // Input loop
    do {
        printf("Enter your choice (1 to put, 2 to get, 0 to quit): ");
        scanf("%d", &choice);
        getchar();  // Clear the newline character

        switch (choice) {
            case 1:  // PUT
                printf("Enter the name: ");
                fgets(message.rd.name, BUFFSIZE, stdin);
                // Remove newline character at the end of the string
                int len = strlen(message.rd.name);
                if (len > 0 && message.rd.name[len - 1] == '\n') {
                    message.rd.name[len - 1] = '\0';
                }

                printf("Enter the id: ");
                scanf("%u", &message.rd.id);
                getchar();  // Clear newline after input

                message.type = PUT;
                write(sock, &message, sizeof(message));
                read(sock, &message, sizeof(message));
                // Output result
                if (message.type == SUCCESS) {
                    printf("Put success.\n");
                } 
                else {
                    printf("Put failed.\n");
                }
                break;

            case 2:  // GET
                printf("Enter the id: ");
                scanf("%u", &message.rd.id);
                getchar();  // Clear newline after input

                message.type = GET;
                write(sock, &message, sizeof(message));
                read(sock, &message, sizeof(message));
                // Output requested data
                if (message.type == SUCCESS) {
                    printf("name: %s\nid: %u\n", message.rd.name, message.rd.id);
                } else {
                    printf("Get failed.\n");
                }
                break;

            case 0:  // QUIT
                break;

            default:
                printf("Invalid choice\n");
        }
    } while (choice != 0);

    close(sock);
    return 0;
}
