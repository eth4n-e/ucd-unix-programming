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

socket_status write_to_socket(int socket_fd, char* buffer, int buf_size) {
    if (buf_size <= 0 || strlen(buffer) > buf_size) {
       errno = EINVAL;
       return SOCKET_INVALID;
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
                return SOCKET_ERROR;
            }
        }
        totWritten += numWritten;
        bufw += numWritten;
    }
    // null terminate written message
    // msg[max_size] = '\0';
    return SOCKET_OK;
}

// user allocates buffer and passes, avoids returning a stack var (char buf[xxxx])
// that will be deallocated when function returns 
// buf_size includes space for null terminator, can only truly fill buffer to buf_size - 1
socket_status read_from_socket(int socket_fd, char* buffer, int buf_size) {
    if (buf_size <= 0 || strlen(buffer) > buf_size) {
        errno = EINVAL;
        return SOCKET_INVALID;
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
            // numRead would only be 0 when connection to socket is closed
            // could not be 0 from reading data b/c then totRead = buf_size
            return SOCKET_CLOSED;
        if (numRead == -1) {
            // continue read process if interrupted
            if (errno == EINTR)
                continue;
            else {
                return SOCKET_ERROR;
            }
        }
        totRead += numRead;
        bufr += numRead;
    }
    // buffer has been filled with help of bufr
    // changes should reflect in actual buffer b/c we took pointer to buffer as arg
    return SOCKET_OK;
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
        goto cleanup;
    }

    int num_questions = quiz -> num_questions;
    if (num_questions <= 0) {
        fprintf(stderr, "Error: quiz size must be greater than 0.\n");
        goto cleanup;
    }

    int num_correct = 0;
    socket_status sock_status;
    // defining answer, response outside loop scope so mem can be freed in cleanup if necessary
    char* answer = NULL;
    char* response = NULL;
    char* results = NULL;
    // iteratively send the client quiz questions
    for (int i = 0; i < num_questions; i++) {
        char* question = quiz -> questions[i];
        sock_status = write_to_socket(socket_fd, question, write_bufsize);
        if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
            fprintf(stderr, "Unable to write to socket.\n");
            goto cleanup;
        }

        answer = malloc(read_bufsize);
        if (answer == NULL) {
            goto cleanup;
        }
        sock_status = read_from_socket(socket_fd, answer, read_bufsize);
        if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
            fprintf(stderr, "Unable to read from socket.\n");
            goto cleanup;
        } else if (sock_status == SOCKET_CLOSED) {
            fprintf(stderr, "Connection to peer closed.\n");
            goto cleanup;
        }
        const char* correct_answer = quiz -> answers[i];
        // allocate a buffer to hold response to send back to client
        response = malloc(write_bufsize);
        if (response == NULL) {
            goto cleanup;
        }
        int compare = strcmp(answer, correct_answer);
        if (compare == 0) {
            num_correct++;
            // snprintf safely copies string into buffer
            snprintf(response, write_bufsize, "Right Answer.");
            sock_status = write_to_socket(socket_fd, response, write_bufsize);
            if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
                fprintf(stderr, "Unable to write to socket.\n");
                goto cleanup;
            }
        } else {
            // snprintf can be used to make parameterized strings
            // explains purpose of using a buffer instead of char* 
            // can create a response that includes correct answer
            snprintf(response, write_bufsize, "Wrong answer. Right answer is %s", correct_answer);
            sock_status = write_to_socket(socket_fd, response, write_bufsize);
            if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
                fprintf(stderr, "Unable to write to socket.\n");
                goto cleanup;
            }
        }
        // de-allocate answer / response after every iteration to ensure I am not overwriting exisitng memory
        free(answer);
        free(response);
        answer = NULL;
        response = NULL;
    }
    results = malloc(write_bufsize);
    if (results == NULL) {
        goto cleanup;
    }
    snprintf(results, write_bufsize, "Your quiz score is %d/%d. Goodbye!\n", num_correct, num_questions);
    sock_status = write_to_socket(socket_fd, results, write_bufsize);
    if (sock_status == SOCKET_INVALID || sock_status == SOCKET_ERROR) {
        fprintf(stderr, "Unable to write to socket.\n");
        goto cleanup;
    }

    // quiz, questions, and answers were dynamically allocated, perform cleanup
    cleanup:
        if (answer) free(answer);
        if (response) free(response);
        if (results) free(results);
        cleanup_quiz(quiz);
    return;
}
