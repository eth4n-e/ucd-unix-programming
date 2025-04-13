#ifndef UTILS_H
#define UTILS_H
#include <sys/socket.h>

/*** METHODS **/
int generate_random_num(int bound);
void display_client_addr(struct sockaddr* addr, socklen_t addr_len);
int write_to_socket(int socket_fd, char* msg, int buf_size);
char* read_from_socket(int socket_fd, char* buffer, int buf_size);
Quiz generate_quiz(char* quiz_q[], char* quiz_a[], int num_questions);
void start_quiz(int socket_fd, Quiz quiz, int write_bufsize, int read_bufsize); 

/*** DATA TYPES ***/
struct Quiz {
  int num_questions;
  char* questions[];
  char* answers[];
};

#endif
