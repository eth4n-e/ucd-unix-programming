#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#define BUFSIZE 16           /* Max message size */
#define BACKLOG 10           /* Pending connection limit */

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

    // bind created socket to struct specifying address 
    int rc = bind(listen_fd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));
    
    if (rc == -1) {
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

    fprintf(stdout, "<Listening on %s:%d>\n", server_ip, port_num);
    fprintf(stdout, "<Press ctrl-C to terminate>\n");

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
        {  
            // obtain the client's address
            // code block limits scope of host, service (only needed for print)
            char host[NI_MAXHOST];
            char service[NI_MAXSERV];
            // getnaminfo is a library function
            // uses socket address structure (client_addr) to find corresponding host / service name
            if(getnameinfo((struct sockaddr*) &client_addr, addr_len, 
                host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
                fprintf(stdout, "Connection from (%s, %s)\n", host, service);

            } else {
                fprintf(stderr, "Connection from (?UNKNOWN?)");
            }
        } 
        // write quiz questions to client
        char outbuf[BUFSIZE];
        size_t totWritten;
        const char* bufw = outbuf;
        // loop ensures write of BUFSIZE
        for (totWritten = 0; totWritten < BUFSIZE; ) {
            // attempt to write entirety of remaining buffer (stored in bufw)
            // write may transfer fewer bytes than requested
            ssize_t numWritten = write(connect_fd, bufw, BUFSIZE - totWritten);
            if (numWritten <= 0) {
                if (numWritten == -1 && errno == EINTR)
                    continue;
                else {
                    fprintf(stderr, "Write error.\n");
                    exit(EXIT_FAILURE);
                }
            }
            totWritten += numWritten;
            bufw += numWritten;
        }
        // read responses from client
        char inbuf[BUFSIZE];
        size_t totRead;
        char* bufr = inbuf;
        // loop ensures read of BUFSIZE bytes
        for (totRead = 0; totRead < BUFSIZE; ) {
            // read() may read fewer bytes than requested
            ssize_t numRead = read(connect_fd, bufr, BUFSIZE - totRead);
            if (numRead == 0)
                break;
            if (numRead == -1) {
                if (errno == EINTR)
                    continue;
                else {
                    fprintf(stderr, "Read error.\n");
                }
            }
            totRead += numRead;
            bufr += numRead;
        }
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
