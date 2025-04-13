#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include "utils.h"
#include "quizdb.h"
#define WRITE_BUFSIZE 512          /* General write message size */
#define READ_BUFSIZE 16            /* Max read message size */
#define BACKLOG 10                 /* Pending connection limit */

// static limits scope to containing file, const for immutability
static const char *preamble =
    "Welcome to Unix Programming Quiz!\n"
    "The quiz comprises five questions posed you one after the other.\n"
    "You have only one attempt to answer a question.\n"
    "Your final score will be sent you after conclusion of the quiz.\n"
    "To start the quiz, press Y and <enter>.\n"
    "To quit the quiz, press q and <enter>.\0";

int main (int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host address> <port number>\n", argv[0]);
        exit(-1);
    }

    char* server_ip = argv[1];
    int port_num = atoi(argv[2]); 

    // Step 1: create socket
    // Domain: AF_INET: IPv4 Address
    // Type: Stream Socket (reliable, consistent, TCP)
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        fprintf(stderr, "socket() error.\n");
        exit(-1);
    }
    // Step 2: bind socket to specified address
    // sockaddr_in is address structure for an AF_INET (IPv4) domain type
    struct sockaddr_in serverAddress;
    // allocate serverAddress the size of the struct
    memset(&serverAddress, 0, sizeof(struct sockaddr_in));
    // sin_family must be AF_INET for a sockaddr_in struct
    serverAddress.sin_family = AF_INET;
    // s_addr is 4-byte num, one byte represents one number in IP address (IPv4 has four 8 bit values)
    // convert string IP to binary IP in network-byte order
    serverAddress.sin_addr.s_addr = inet_addr(server_ip);
    // convert port_num into network byte order
    // big-endian, used by network protocols, ensures consistent data interpretation / format
    serverAddress.sin_port = htons(port_num);

    // bind created socket to address specified by struct
    int ret_code = bind(listen_fd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));
    
    if (ret_code == -1) {
        fprintf(stderr, "bind() error.\n");
        exit(-1);
    }
    // Step 3: listen to incoming connections
    // backlog limits number of pending connections
    // a client that attempts to connect before server invokes accept is pending
    // kernel maintains queue of pending connections
    if (listen(listen_fd, BACKLOG) == -1) {
        exit(EXIT_FAILURE);
    }

    printf("<Listening on %s:%d>\n", server_ip, port_num);
    printf("<Press ctrl-C to terminate>\n");

    // Step 4: accept incoming connections 
    for (;;) {
        struct sockaddr_storage client_addr;
        socklen_t addr_len = sizeof(struct sockaddr_storage);
        // accept incoming connection from client
        // creates a new socket, used to stream data between server & client
        // listening socket (listen_fd) remains open to accept incoming connections
        // client_addr & addr_len return address of peer socket (one invoking connect - client)
        int connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);

        if (connect_fd == -1) {
            fprintf(stderr, "accept() error.\n");
        }

        // display client address
        display_client_addr((struct sockaddr*)&client_addr, addr_len);

        /* TO-DO:
        - begin quiz with preamble statement
        - wait for client input
            - start quiz if client sends Y, else close connection
        - loop
            - server issues five quiz questions
            - for each question
                - send right answer if client correct
                - indicate to client they are wrong if incorrect and send right answer
        - conclusion of quiz
            - send quiz results to client
            - close connection and serve next client
        */
        // write preamble to socket, handle invalid buffer size
        if (write_to_socket(connect_fd, preamble, WRITE_BUFSIZE) == -1) {
            fprintf(stderr, "Invalid buffer size for write.\n");
            exit(EXIT_FAILURE);
        }

        // client instructed to enter 'Y' or 'q', grab first character only
        // note: client side logic ensures only one character sent (no whitespace)
        char read_buf[READ_BUFSIZE];
        char* response = read_from_socket(connect_fd, read_buf, READ_BUFSIZE);
        // response to preamble determines client's interest in quiz
        switch(tolower(response[0])) {
            case 'y':
                //user agrees to quiz, pass socket and buffer sizes for communication with client
                const int quiz_size = sizeof(QuizQ) / sizeof(QuizQ[0]);
                start_quiz(connect_fd, QuizQ, quiz_size, WRITE_BUFSIZE, READ_BUFSIZE);
                break;
            case 'q':
                // user does not want quiz, close connection socket
                if (close(connect_fd) == -1) {
                    fprintf(stderr, "Close error.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                break;
        }
        // write quiz questions to client
        // close connection socket        
        if (close(connect_fd) == -1) {
            fprintf(stderr, "Close error.\n");
            exit(EXIT_FAILURE);
        }
    }
    // close listening socket
    if (close(listen_fd) == -1) {
        fprintf(stderr, "Close error.\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
