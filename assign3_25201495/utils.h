#ifndef UTILS_H
#define UTILS_H
#include <sys/socket.h>

int generate_random_num(int bound);
void display_client_addr(struct sockaddr* addr, socklen_t addr_len);
int write_to_socket(int socket_fd, char* msg, int buf_size);
char* read_from_socket(int socket_fd, char* buffer, int buf_size);
void start_quiz(int socket_fd, char* quiz[], int quiz_size, int write_bufsize, int read_bufsize); 
void output_preamble();

#endif
