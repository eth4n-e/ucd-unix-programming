#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "utils.h"
#define NUM_QUIZ_QUESTIONS 5    /* Number of quiz questions */

/*** SERVER ***/
int generate_random_num(int bound) {
    if (bound > 0) {
        return (rand() % bound) + 1;
    }

    fprintf(stderr, "Bound must exceed 0.\n");
    return -1;
}

void display_client_addr(struct sockaddr* addr, socklen_t addr_len) {
    // obtain the client's address
    // code block limits scope of host, service (only needed for print)
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    // getnaminfo is a library function
    // uses socket address structure (client_addr) to find corresponding host / service name
    if(getnameinfo(addr, addr_len, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
        fprintf(stdout, "Connection from (%s, %s)\n", host, service);
    } else {
        fprintf(stderr, "Connection from (?UNKNOWN?)");
    }
}

int write_to_socket(int socket_fd, char* msg, int buf_size) {
    if (buf_size <= 0) {
       errno = EINVAL;
       return -1;
    }

    size_t totWritten;
    // store message to write in temporary pointer
    // allows us to modify position of bufw (bufw += numWritten)
    // w/o losing reference to start of msg
    const char* bufw = msg;
     
    // loop ensures write of buf_size
    for (totWritten = 0; totWritten < buf_size; ) {
        // attempt to write entirety of remaining buffer (stored in bufw)
        // write may transfer fewer bytes than requested
        ssize_t numWritten = write(socket_fd, bufw, buf_size - totWritten);
        if (numWritten <= 0) {
            // continue write process if error caused by interruption
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
    
    return 0;
}

// user allocates buffer and passes, avoids returning a stack var (char buf[xxxx])
// that will be deallocated when function returns 
char* read_from_socket(int socket_fd, char* buffer, int buf_size) {
    if (buf_size <= 0) {
        errno = EINVAL;
        return NULL;
    }

    size_t totRead;
    // use temp pointer to read data into buffer
    // buffer preserves starting address of data
    char* bufr = buffer;
    // loop ensures read of buf_size bytes
    for (totRead = 0; totRead < buf_size; ) {
        // read() may read fewer bytes than requested
        ssize_t numRead = read(socket_fd, bufr, buf_size - totRead);
        if (numRead == 0)
            break;
        if (numRead == -1) {
            // continue read process if interrupted
            if (errno == EINTR)
                continue;
            else {
                fprintf(stderr, "Read error.\n");
            }
        }
        totRead += numRead;
        bufr += numRead;
    }
    
    printf("Received message: %s\n", buffer);
    // data read into inbuf, using bufr to move along buffer
    return buffer;
}

/* Idea for refactoring
- create a struct that represents a quiz
    - question arr
    - answer arr
    - size (num_questions)
- create a method to generate a quiz
- call this method from start_quiz or call in server and pass quiz struct to start_quiz
*/
void start_quiz(int socket_fd, char* quiz[], int quiz_size, int write_bufsize, int read_bufsize) {
    if (quiz_size <= 0) {
        fprintf(stderr, "Error: quiz size must be greater than 0.\n");
        return;
    }

    // iteratively send the client quiz questions
    for (int i = 0; i < NUM_QUIZ_QUESTIONS; i++) {
        int question_idx = generate_random_num(quiz_size);
        if (question_idx == -1) {
            fprintf(stderr, "Failed to generate a random number.\n");
            return;
        }
        
        char* question = quiz[question_idx];

        if (write_to_socket(socket_fd, question, write_bufsize) == -1) {
            fprintf(stderr, "Invalid buffer size for write.\n");
            exit(EXIT_FAILURE);
        }

        char read_buf[read_bufsize];
        const char* usr_answer = read_from_socket(socket_fd, read_buf, read_bufsize);
        printf("User's reponse to question %d: %s\n", question_idx, usr_answer); 
        /* const char* correct_answer = QuizA[question_idx];

        // allocate a buffer to hold response to send back to client
        char response[write_bufsize];

        if (strcmp(usr_answer, correct_answer) == 0) {
            // snprintf safely copies string into buffer
            snprintf(response, sizeof(response), "Right Answer.");
            if (write_to_socket(socket_fd, response, write_bufsize) == -1) {
                fprintf(stderr, "Invalid buffer size for write.\n");
                exit(EXIT_FAILURE);
            }
        } else {
            // snprintf can be used to make parameterized strings
            // explains purpose of using a buffer instead of char* 
            // can create a response that includes correct answer
            snprintf(response, sizeof(response), "Wrong answer. Right answer is %s", correct_answer);
        } */
    }
    return;
}
/*** SERVER ***/
