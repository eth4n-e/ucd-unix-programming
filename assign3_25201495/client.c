#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include "utils.h"
#define READ_BUFSIZE 512    /* Max message size sent from server */
#define WRITE_BUFSIZE 16    /* Max message size to send to server */

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage %s <host address> <port number>\n", argv[0]);
        exit(-1);
    }

    const char* server_ip = argv[1];
    const int port_num = atoi(argv[2]);

    // Step 1: create new socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        fprintf(stderr, "socket() error.\n");
        exit(-1);
    }

    // Step 2: connect socket to server, similar to server bind phase 
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(server_ip);
    serverAddress.sin_port = htons(port_num);

    // connect client socket to listening socket by address
    // address of listening socket specified by serverAddress & addrlen (sizeof(...))
    int ret_code = connect(client_fd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));
    
    if (ret_code == -1) {
        fprintf(stderr, "connect() error.\n");
        exit(-1);
    }

    // code block to handle accepting / denying quiz start
    // want to refactor this into a more appropriate method
    {
        // read preamble from server
        char read_buf[READ_BUFSIZE];
        char* msg = read_from_socket(client_fd, read_buf, READ_BUFSIZE);
        if (msg == NULL) {
            fprintf(stderr, "Error reading from socket.\n");
            exit(-1);
        }

        printf("%s\n", msg);

        char *response = NULL;
        size_t len = 0;
        if(getline(&response, &len, stdin) != -1) {
            char first = *response;
            if (write_to_socket(client_fd, &first, WRITE_BUFSIZE) == -1) {
                fprintf(stderr, "Invalid buffer size for write.\n");
                exit(EXIT_FAILURE);
            }
            
            // close the client's socket if they do not want to take quiz
            if (first == 'q') {
                if (close(client_fd) == -1) {
                    fprintf(stderr, "Close error.\n");
                    exit(EXIT_FAILURE);
                }
                // mark connection as closed
                client_fd = -1;
                free(response);
                exit(EXIT_SUCCESS);
            }
        }
    }

    // data transfer loop follows same logic as server
    // loop handles quiz interaction between server
    for(;;) {
        char bufr[READ_BUFSIZE];
        char* question = read_from_socket(client_fd, bufr, READ_BUFSIZE);
        if (question == NULL) {
            fprintf(stderr, "Error reading from socket.\n");
            exit(-1);
        }
    
        printf("%s\n", question);

        char* answer = NULL;
        size_t len;
        if (getline(&answer, &len, stdin) != -1) {
            if (write_to_socket(client_fd, answer, WRITE_BUFSIZE) == -1) {
                fprintf(stderr, "Invalid buffer size for write.\n");
                exit(EXIT_FAILURE);
            }
        }
        // for spacing purposes
        printf("\n");
    }

    if (close(client_fd) == -1) {
        fprintf(stderr, "Close error.\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
