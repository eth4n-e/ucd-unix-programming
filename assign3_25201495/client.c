#include <stdio.h>
#include <sys/socket.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage %s <host address> <port number>\n", argv[0]);
        exit(-1);
    }

    const char* server_ip = argv[1];
    const int port_num = argv[2];

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

    // connect client to listening socket
    // address of listening socket specified by serverAddress & addrlen (sizeof(...))
    int rc = connect(client_fd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));
    
    if (rc == -1) {
        fprintf(stderr, "connect() error.\n");
        exit(-1);
    }

    // data transfer loop follows same logic as server
    for(;;) {
        
    }
}
