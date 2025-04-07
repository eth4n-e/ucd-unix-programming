#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#define BUFSIZE 16           /* Max message size */

int main (int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Server program expects two arguments: IPv4 Address, Port Number");
    }

    char* server_ip = argv[0];
    int port_num = atoi(argv[1]); 

    // Step 1: create socket
    // Domain: AF_INET: IPv4 Address
    // Type: Stream Socket (reliable, consistent, TCP)
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
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
    // convert string IP to numeric IP
    serverAddress.sin_addr.s_addr = inet_addr(server_ip);
    // convert port_num into network byte order
    // big-endian, used by network protocols, ensures consistent data interpretation / format
    serverAddress.sin_port = htons(port_num);

    //
    int rc = bind(socket_fd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));
    
    if (rc == -1) {
        fprint(stderr, "bind() error.\n");
        exit(-1);
    }
    // Step 3: listen to incoming connections
    // Step 4: accept incoming connections 
    // Step 5: establish connection with client
}
