#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
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
        exit(EXIT_FAILURE);
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

    socket_status sock_status;
    // code block to handle accepting / denying quiz start
    // want to refactor this into a more appropriate method
    {
        // read preamble from server
        char read_buf[READ_BUFSIZE];
        sock_status = read_from_socket(client_fd, read_buf, READ_BUFSIZE);
        if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
            fprintf(stderr, "Unable to read from socket.\n");
            exit(EXIT_FAILURE);
        } else if (sock_status == SOCKET_CLOSED) {
            goto close_connection_to_server;
        }

        printf("%s\n", read_buf);

        char *response = NULL;
        size_t len = 0;
        if(getline(&response, &len, stdin) != -1) {
            char first = *response;
            sock_status = write_to_socket(client_fd, &first, WRITE_BUFSIZE);
            if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
                fprintf(stderr, "Unable to write to socket.\n");
                exit(EXIT_FAILURE);
            }
            
            // close the client's socket if they do not want to take quiz
            if (first == 'q') {
                free(response);
                goto close_connection_to_server;
            }
            printf("\n");
            free(response);
        }
    }

    // data transfer loop follows same logic as server
    // loop handles quiz interaction between server
    for(;;) {
        // initialize a file descriptor set to monitor client_fd and / or stdin
        fd_set readfds;
        FD_ZERO(&readfds); // initialize set
        FD_SET(client_fd, &readfds); // add client_fd to readfds set
        FD_SET(STDIN_FILENO, &readfds); // add stdin to readfds set
       
        // determine the largest fd
        int max_fd = (client_fd > STDIN_FILENO) ? client_fd : STDIN_FILENO;
        // select(int num_fds, read_set, ...)
        // method checks the first num_fds [0, max_fd] in each set
        // select method blocks program execution until the fds are ready
        // ... select will unblock when: data arrives to fd being watched, socket closes, etc.
        // returns the number of 'ready' fds (e.g. available to be read from, written to, etc.)
        int ready = select(max_fd+1, &readfds, NULL, NULL, NULL);
        if (ready == -1) {
            fprintf(stderr, "Unable to perform select.\n");
            exit(EXIT_FAILURE);
        }
        
        // select updates the set to include only those fds ready for an operation
        // FD_ISSET(fd, set) checks if fd is in set (equivalent to being ready)
        // select removed need for two read checks b/c select will catch when client_fd 
        // is ready to be read from and add it to readfds. And read checks had same structure
        if (FD_ISSET(client_fd, &readfds)) {
            char bufr[READ_BUFSIZE];
            sock_status = read_from_socket(client_fd, bufr, READ_BUFSIZE);
            printf("read status: %d\n", sock_status);
            if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
                fprintf(stderr, "Unable to read from socket.\n");
                exit(EXIT_FAILURE);
            } else if (sock_status == SOCKET_CLOSED) {
                goto close_connection_to_server;
            }
    
            printf("%s\n", bufr);
        }
        
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char* answer = NULL;
            size_t len;
            if (getline(&answer, &len, stdin) != -1) {
                // replace newline character with null terminator
                // newline interferes with strcmp on server
                // strcspn returns length of string up to first occurrence of string arg
                answer[strcspn(answer, "\n")] = '\0';
                sock_status = write_to_socket(client_fd, answer, WRITE_BUFSIZE);
                if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
                    fprintf(stderr, "Unable to write to socket.\n");
                    free(answer);
                    exit(EXIT_FAILURE);
                }
            }
            // reminder that getline dynamically allocates answer, my responsibility to free
            free(answer);
        }
    }

    close_connection_to_server:
        if (close(client_fd) == -1) {
            fprintf(stderr, "Close error.\n");
            exit(EXIT_FAILURE);
        }
    exit(EXIT_SUCCESS);
}
