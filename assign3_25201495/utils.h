#ifndef UTILS_H
#define UTILS_H
#include <sys/socket.h>

int generate_random_num(int bound);
void display_client_addr(struct sockaddr* addr, socklen_t addr_len);
int write_to_client(int connect_fd, const char* msg, int buf_size);
int read_from_client(int connect_fd, int buf_size);
void output_preamble();

#endif
