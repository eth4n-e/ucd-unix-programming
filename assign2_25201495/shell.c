#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "helpers.h"
#include "signal_handlers.h"

/*
Name: Ethan Epperson
Student Number: 25201495
Email: ethan.epperson@ucdconnect.ie
*/

/* Notes
- Define one header file for useful functions, perhaps signal handlers, error handling, etc.
- use one c source file to implement these function definitions
- the other c source file should be for the main program
*/

int main (int argc, char **argv) {
    setup_signal_handlers();

    FILE *input = stdin;
    // to store inputted line
    char *line = NULL;
    // default buffer allocation
    size_t len = 0;
    // number characters read
    ssize_t read;
    while(1) {
        // print prompt before child process created
        display_prompt();
        fflush(stdout);    

        read = getline(&line, &len, input);
        char *command;
        char **args;
        // getline returns -1 on EOF or error
        if (read == -1) {
            printf("\nEOF received, exiting process.\n");
            exit(0);
        } else {
            args = parse_and_store_tokens(line);
            command = args[0];
            // check for cd in parent process
            // cd is a shell built-in not an external executable program
            // running in child process would fail as directory changes
            //  would only be reflected in that process
            if (strcmp("cd", command) == 0) {
                printf("In change directory block\n");
                fflush(stdout);
                execute_chdir(args);
            }
        }

        // variables for process management
        pid_t child_pid;
        int child_status;

        // spawn a new process
        child_pid = fork();    
    
        // 0 is returned to child processes
        if (child_pid == 0) {
            execvp(command, args);
            printf("Error: unknown command\n");
            exit(0);
        } else {
            // parent process
            // wait for command to finish (child process) before prompting for next command
            wait(&child_status);
        }
    }

    return 0;
}
