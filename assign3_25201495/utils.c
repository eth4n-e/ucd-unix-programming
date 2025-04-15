#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include "utils.h"
#define NUM_QUIZ_QUESTIONS 5    /* Number of quiz questions */

/*** SERVER ***/
int generate_random_num(int bound) {
    if (bound <= 0) {
        fprintf(stderr, "Bound must exceed 0.\n");
        return -1;
    }

    return rand() % bound;
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

// need to add null terminator to buffer
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
            break; // end of data
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
    
    // data read into inbuf, using bufr to move along buffer
    return buffer;
}

// more intuitive to return a pointer because
// 1. dynamically allocating memory for questions / answers based on num_questions
// 2. size of questions / answers is not fixed 
// 3. returning pointer avoids copying large amounts of data
// 4. arrays defined dynamically won't copy correctly via shallow copy
// returning structs by value works for small, fixed-sized structs
Quiz* generate_quiz(char* quiz_q[], char* quiz_a[], int quiz_size, int num_questions) {
    if (num_questions <= 0 || num_questions > quiz_size) {
        fprintf(stderr, "Quiz cannot be defined with %d questions.\n", num_questions);
        return NULL;
    }

    Quiz* quiz = malloc(sizeof(Quiz));
    quiz -> num_questions = num_questions;
    // questions / answers point to a list of strings
    // must allocate a string (char*) for each question
    quiz -> questions = malloc(num_questions * sizeof(char*));
    if (quiz -> questions == NULL) {
        fprintf(stderr, "Unable to allocate questions.\n");
        return NULL;
    }
    quiz -> answers = malloc(num_questions * sizeof(char*));
    if (quiz -> answers == NULL) {
        fprintf(stderr, "Unable to allocate questions.\n");
        return NULL;
    }

    // seed based on current time before generating random nums
    srand(time(NULL));

    // add questions / answers to our quiz
    for (int i = 0; i < num_questions; i++) {
        int question_idx = generate_random_num(quiz_size);
        // no dynamic allocation here because each question / answer is predefined from QuizQ/A
        quiz -> questions[i] = quiz_q[question_idx];
        quiz -> answers[i] = quiz_a[question_idx];
    }
    
    return quiz;
}

void cleanup_quiz(Quiz* quiz) {
    if (quiz == NULL) {
        return;
    }

    // free all dynamically allocated variables
    // Note: don't free quiz -> questions[i] b/c each char* points to a string literal 
    // not dynamically allocated memory
    free(quiz -> questions);
    free(quiz -> answers);
    free(quiz);
}

void start_quiz(int socket_fd, Quiz* quiz, int write_bufsize, int read_bufsize) {
    if (quiz == NULL) {
        fprintf(stderr, "Unable to begin non-existent quiz.\n");
        cleanup_quiz(quiz);
        return;
    }

    int num_questions = quiz -> num_questions;
    if (num_questions <= 0) {
        fprintf(stderr, "Error: quiz size must be greater than 0.\n");
        cleanup_quiz(quiz); 
        return;
    }

    // iteratively send the client quiz questions
    for (int i = 0; i < num_questions; i++) {
        char* question = quiz -> questions[i];

        if (write_to_socket(socket_fd, question, write_bufsize) == -1) {
            fprintf(stderr, "Invalid buffer size for write.\n");
            cleanup_quiz(quiz);
            exit(EXIT_FAILURE);
        }

        char read_buf[read_bufsize];
        const char* usr_answer = read_from_socket(socket_fd, read_buf, read_bufsize);
        const char* correct_answer = quiz -> answers[i];

        // allocate a buffer to hold response to send back to client
        char response[write_bufsize];
        
        printf("User answer: %s", usr_answer);
        printf("Correct answer: %s\n", correct_answer);

        if (strcmp(usr_answer, correct_answer) == 0) {
            // snprintf safely copies string into buffer
            snprintf(response, sizeof(response), "Right Answer.");
            if (write_to_socket(socket_fd, response, write_bufsize) == -1) {
                fprintf(stderr, "Invalid buffer size for write.\n");
                cleanup_quiz(quiz);
                exit(EXIT_FAILURE);
            }
        } else {
            // snprintf can be used to make parameterized strings
            // explains purpose of using a buffer instead of char* 
            // can create a response that includes correct answer
            snprintf(response, sizeof(response), "Wrong answer. Right answer is %s", correct_answer);
            if (write_to_socket(socket_fd, response, write_bufsize) == -1) {
                fprintf(stderr, "Invalid buffer size for write.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
   
    // quiz, questions, and answers were dynamically allocated, perform cleanup
    cleanup_quiz(quiz);
    return;
}
/*** SERVER ***/
