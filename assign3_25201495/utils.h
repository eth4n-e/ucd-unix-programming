#ifndef UTILS_H
#define UTILS_H
#include <sys/socket.h>

/*** DATA TYPES ***/
// defining a struct using typedef creates an alias (Quiz) for the struct
// can omit struct keyword when using struct as return type, param, var, etc.
typedef struct {
  int num_questions;
  char** questions;
  char** answers;
} Quiz;

/*** METHODS **/
int generate_random_num(int bound);
void display_client_addr(struct sockaddr* addr, socklen_t addr_len);
int write_to_socket(int socket_fd, char* buffer, int buf_size);
char* read_from_socket(int socket_fd, char* buffer, int buf_size);
void generate_unique_questions(Quiz* quiz, char* quiz_q[], char* quiz_a[], int quiz_set_size);
Quiz* generate_quiz(char* quiz_q[], char* quiz_a[], int quiz_set_size, int num_questions);
int is_in_quiz_set(int q_set[], int set_size, int q_num);
void cleanup_quiz(Quiz* quiz);
void start_quiz(int socket_fd, Quiz* quiz, int write_bufsize, int read_bufsize); 

#endif
