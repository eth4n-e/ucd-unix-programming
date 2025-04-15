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

int write_to_socket(int socket_fd, char* buffer, int buf_size) {
    if (buf_size <= 0) {
       errno = EINVAL;
       return -1;
    }

    size_t totWritten;
    // store message to write in temporary pointer
    // allows us to modify position of bufw (bufw += numWritten)
    // w/o losing reference to start of msg
    const char* bufw = buffer;
    // int max_size = buf_size - 1;
    // loop ensures write of max_size
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
    // null terminate written message
    // msg[max_size] = '\0';
    return 0;
}

// user allocates buffer and passes, avoids returning a stack var (char buf[xxxx])
// that will be deallocated when function returns 
// buf_size includes space for null terminator, can only truly fill buffer to buf_size - 1
char* read_from_socket(int socket_fd, char* buffer, int buf_size) {
    if (buf_size <= 0) {
        errno = EINVAL;
        return NULL;
    }
    // clear buffer before reading data into it
    // ensures no residual data remains that could interfere
    // filling with 0 (null byte, byte with val = 0) ensures any unfilled data in buffer
    // is not stray / random, null terminator is same a null byte marking end of string
    memset(buffer, 0, buf_size);

    size_t totRead;
    // use temp pointer to read data into buffer
    // buffer preserves starting address of data
    char* bufr = buffer;
    // loop ensures read of buf_size bytes
    for (totRead = 0; totRead < buf_size; ) {
        // read() may read fewer bytes than requested
        ssize_t numRead = read(socket_fd, bufr, buf_size - totRead);
        if (numRead == 0)
            // wondering if I should return something to indicate EOF or closed connection
            // from what I understand numRead would only be 0 when connection to socket is closed
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

int is_in_quiz_set(int q_set[], int set_size, int q_num) {
    int found = 0;
    for (int i = 0; i < set_size; i++) {
        if (q_num == q_set[i]) {
            found = 1;
            return found;
        }
    }
    return found;
}

// Clarification:
// num_questions: refers to number of questions to include in Quiz
// quiz_size: size of original quiz set that we are drawing from
void generate_unique_questions(Quiz* quiz, char* quiz_q[], char* quiz_a[], int quiz_set_size) {
    if (quiz == NULL || quiz -> num_questions <= 0 || quiz -> num_questions > quiz_set_size) {
        fprintf(stderr, "Unable to generate questions for an invalid quiz.\n");
        return;
    }

    int num_questions = quiz -> num_questions;
    int quiz_set[num_questions];
    for (int i = 0; i < num_questions; i++) {
        // define variables outside loop, scope ends with } brace closing do block
        // thus variable inside do block not usable in while condition
        int question_idx;
        int check_until;
        // continue to generate questions until it is unique
        do {
            question_idx = generate_random_num(quiz_set_size);
            quiz_set[i] = question_idx;
            // check values before most recently added
            // checking most recently added would result in an erroneous duplicate flag
            check_until = i-1; 
        } while (is_in_quiz_set(quiz_set, check_until, question_idx));

        // add question to quiz once index is unique
        quiz -> questions[i] = quiz_q[question_idx];
        quiz -> answers[i] = quiz_a[question_idx];
    }

    return;
}

// more intuitive to return a pointer because
// 1. dynamically allocating memory for questions / answers based on num_questions
// 2. size of questions / answers is not fixed 
// 3. returning pointer avoids copying large amounts of data
// 4. arrays defined dynamically won't copy correctly via shallow copy
// returning structs by value works for small, fixed-sized structs
Quiz* generate_quiz(char* quiz_q[], char* quiz_a[], int quiz_set_size, int num_questions) {
    if (num_questions <= 0 || num_questions > quiz_set_size) {
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
    generate_unique_questions(quiz, quiz_q, quiz_a, quiz_set_size);
    
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
    printf("Num questions: %d\n", num_questions);
    if (num_questions <= 0) {
        fprintf(stderr, "Error: quiz size must be greater than 0.\n");
        cleanup_quiz(quiz); 
        return;
    }

    int num_correct = 0;

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
        int compare = strcmp(usr_answer, correct_answer);
        // comparisons have been off b/c usr_answer includes enter key which maps to ASCII val of 10
        printf("Result of comparing answers: %d\n", compare);
        if (compare == 0) {
            num_correct++;
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
    printf("End of quiz\n");
    char results[write_bufsize];
    snprintf(results, sizeof(results), "Your quiz szore if %d/%d. Goodbye!\n", num_correct, num_questions);
    if (write_to_socket(socket_fd, results, write_bufsize) == -1) {
        fprintf(stderr, "Invalid buffer size for write.\n");
        exit(EXIT_FAILURE);
    }
    cleanup_quiz(quiz);
    return;
}
/*** SERVER ***/
