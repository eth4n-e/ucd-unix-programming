#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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

static char* prompt = "# ";

int main (int argc, char **argv) {
    FILE *input = stdin;
    // to store inputted line
    char *line = NULL;
    // default buffer allocation
    size_t len = 0;
    // number characters read
    ssize_t read;
    while(1) {
        // print prompt before child process created
        printf("%s", prompt);
        fflush(stdout);    

        // variables for process management
        pid_t child_pid;
        int child_status;

        // spawn a new process
        child_pid = fork();    
    
        // 0 is returned to child processes
        if (child_pid == 0) {
            // execute command receive
            // execvp(command, parsed_arguments)
            // prevent the child process from executing before prompt
            // sleep(5);
            read = getline(&line, &len, input);
            char *command;
            char **args = malloc(sizeof(char*));
            int count = 0;
            // getline returns -1 on EOF or error
            if (read != -1) {
                // strcspn finds index of first occurrence of \n, otherwise returns length of string
                // if \n exists, replace that character with 0 or null terminator to end string
                line[strcspn(line, "\n")] = 0;
                // strtok parses inputted string, delimited by 2nd arg into tokens
                char *token = strtok(line, " ");
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
                command = args[0];
                execvp(command, args);
            }
            printf("Unknown command\n");
            exit(0);
        } else {
            // parent process
            // should wait for command to finish (child process) before prompting for next command
            wait(&child_status);
            printf("%s", prompt);
            fflush(stdout);    
        }
    }

    return 0;
}
