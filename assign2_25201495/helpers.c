#include "helpers.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#define DATE_SIZE 12

/*
Name: Ethan Epperson
Student Number: 25201495
Email: ethan.epperson@ucdconnect.ie
*/

void display_prompt() {
    char CURR_TIME[DATE_SIZE];     
    struct tm *time_info; 

    time_info = get_current_time();
    
    // strftime formats a tm struct and stores in char[]
    strftime(CURR_TIME, sizeof(CURR_TIME), "%d/%m %H:%M", time_info);

    printf("[%s]# ", CURR_TIME);    
    // want print to run immediately
    fflush(stdout); 
}

struct tm* get_current_time() {
    time_t seconds;
    // time function returns seconds since 00:00:00 UTC, January 1, 1970
    // parameter is pointer to time_t object to store time
    // function also returns the time as a time_t object
    seconds = time(NULL);

    struct tm *time_info;
    // localtime generates a pointer to a tm structure with arg calendar time
    time_info = localtime(&seconds);

    return time_info;
}

char** parse_and_store_tokens(char *line) {
    int count = 0;
    char **args = malloc(sizeof(char*));

    // strcspn finds index of first occurrence of \n, otherwise returns length of string
    // if \n exists, replace that character with 0 or null terminator to end string
    line[strcspn(line, "\n")] = 0;
    // strtok parses inputted string, delimited by 2nd arg into tokens
    char *token = strtok(line, " ");
    // store each argument in line
    while(token != NULL) {
        args[count] = token;
        count++;
        args = realloc(args, (count + 1) * sizeof(char*));
        token = strtok(NULL, " ");
    }
    count++;
    // add an extra slot to hold a char*
    args = realloc(args, (count + 1) * sizeof(char*));
    // insert NULL into extra slot, execvp expects NULL terminated array of character pointers
    // NULL pointer tells execvp when to stop reading arguments
    args[count] = NULL;
    

    return args;
}

// static keyword sets scope to containing file
static size_t count_strings(char **arr) {
    // counts number of strings in a null terminated char **
    // only utilized with null terminated args resulting from parse_and_store_tokens
    size_t count = 0;
    while (arr[count] != NULL) {
        count++;
    }

    return count;
}

void execute_chdir(char **args) {
    int result;
    char *path;
    size_t arg_size = count_strings(args);
    if (arg_size < 2) {
        // change to path given by env variable HOME if no path specified
        // cd called without a path does nothing in bash
        // search for "HOME" env var
        path = getenv("HOME");
        if (path) {
            printf("Path: %s\n", path);
            fflush(stdout);
            result = chdir(path);
            printf("Result: %d\n", result);
            fflush(stdout);
        } else {
            perror("Unable to find HOME environment variable");
        }
        return;
    } else {
        path = args[1];
        printf("Path: %s\n", path);
        fflush(stdout);
        result = chdir(path);
        printf("Result: %d\n", result);
        fflush(stdout);
    } 

    // chdir returns -1 on failure
    if (result == -1) {
        perror("Unable to change directory");
    }
}
