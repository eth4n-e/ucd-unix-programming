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
#define NUM_QUIZ_QUESTIONS 5       /* Generating 5 question quiz */

// static limits scope to containing file, const for immutability
// had to define as static, fixed-size array instead of char*
// reason: added null terminator character to write_to / read_from socket
// preamble < 512 bytes so with char* approach write_to_socket was trying to access mem past
// the literal (not valid for reading)
static char preamble[WRITE_BUFSIZE] = 
    "Welcome to Unix Programming Quiz!\n"
    "The quiz comprises five questions posed you one after the other.\n"
    "You have only one attempt to answer a question.\n"
    "Your final score will be sent you after conclusion of the quiz.\n"
    "To start the quiz, press Y and <enter>.\n"
    "To quit the quiz, press q and <enter>.";

int main (int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host address> <port number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* server_ip = argv[1];
    int port_num = atoi(argv[2]); 

    // Step 1: create socket
    // Domain: AF_INET: IPv4 Address
    // Type: Stream Socket (reliable, consistent, TCP)
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        fprintf(stderr, "socket() error.\n");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
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
            goto close_connection_to_client;
        }

        // display client address
        display_client_addr((struct sockaddr*)&client_addr, addr_len);
        socket_status sock_status;
        // write preamble to socket, handle invalid buffer size
        sock_status = write_to_socket(connect_fd, preamble, WRITE_BUFSIZE);
        if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
            fprintf(stderr, "Unable to write to socket.\n");
            goto close_connection_to_client; 
        }

        // client instructed to enter 'Y' or 'q', grab first character only
        // note: client side logic ensures only one character sent (no whitespace)
        char response[READ_BUFSIZE];
        sock_status = read_from_socket(connect_fd, response, READ_BUFSIZE);
        if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
            fprintf(stderr, "Unable to read from socket.\n");
            goto close_connection_to_client;
        } else if (sock_status == SOCKET_CLOSED) {
            // using goto to jump to closing connection socket
            // a SOCKET_CLOSED status trying to read from client indicates client ended connection
            goto close_connection_to_client;
        }
        // response to preamble determines client's interest in quiz
        switch(tolower(response[0])) {
            // expected expression errors b/c case labels are not their own scope (fall under switch scope)
            // so if quiz_size was also defined in case 'q', the definitions would collide
            // in C the next statement following a case label must be executable unless in new scope {}
            case 'y': {
                //user agrees to quiz, pass socket and buffer sizes for communication with client
                int quiz_size = get_quiz_size();
                char** quiz_questions = get_quiz_questions();
                char** quiz_answers = get_quiz_answers();
                Quiz* quiz = generate_quiz(quiz_questions, quiz_answers,
                    quiz_size, NUM_QUIZ_QUESTIONS);
                start_quiz(connect_fd, quiz, WRITE_BUFSIZE, READ_BUFSIZE);
                break;
            }
            case 'q': {
                // user does not want quiz, close connection socket
                goto close_connection_to_client;
                break;
            }
            default:
                break;
        }
        // close socket connection to client
        close_connection_to_client:
            printf("Closing connection to client.\n");
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
