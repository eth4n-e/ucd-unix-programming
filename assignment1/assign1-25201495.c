#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/*
NAME: Ethan Epperson
STUDENT-NUMBER: 25201495
EMAIL: ethan.epperson@ucdconnect.ie

PROGRAM DESCRIPTION
- Program implements head and even functionality

SUCCESS OF PROGRAM
- 
*/

// create structs for: version info, set of options
struct Version {
    char *name; 
    char *email;
    int student_num;
};

struct Version info = {"Ethan Epperson", "ethan.epperson@ucdconnect.ie", 25201495};

void print_version_info() {
    printf("Student Name: %s\n", info.name);
    printf("Student Email: %s\n", info.email);
    printf("Student Number: %d\n", info.student_num);
    return;
}

void print_usage(const char *program_name) {
    printf("Usage: %s [options] ... [file]\n", program_name);
    printf("Options:\n");
    printf("    -n K        Output first K lines\n"); 
    printf("    -V          Output version info\n");
    printf("    -h          Display usage\n");
    printf("    -e          Print even lines\n");
    return;
}

void head(FILE *input, int num_lines, bool even) {
    // using an array of pointers to store lines
    // allows me to receive all of users input before printing in case of stdin
    char **lines = malloc(num_lines * sizeof(char *));
    if(!lines) {
        perror("Memory allocation failed");
        return;
    }    

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0;
    int curr_lin = 1; // use curr_lin like a pointer to current line
   
    if (input == stdin) {
        printf("Input up to %d lines to stdin: \n", num_lines);
    }
 
    while (count < num_lines && (read = getline(&line, &len, input)) != -1) {
       // print even lines if specified otherwise each line  
        if (even) {
            if ((curr_lin % 2) == 0) {
                lines[count] = strdup(line);
                if(!lines[count]) {
                    perror("Memory allocation failed");
                    break;
                }
                count++;
            }
        } else {
            lines[count] = strdup(line);
            if(!lines[count]) {
                perror("Memory allocation failed");
                break;
            }
            count++;
        }     
        curr_lin += 1;
    }
        
    printf("\nOutput: \n");
    for (int i = 0; i < count; i++) {
        printf("%s", lines[i]);
        // free memory of pointer to a specific line
        free(lines[i]);
    }
    // free memory storing array of char*
    free(lines);
}

int main(int argc, char *argv[]) {
    const char *prog_name = argv[0];
    if (argc < 2) {
        print_usage(prog_name);
        return 0;
    }
    FILE *input;
    // defaults for head program
    int num_lines = 10;
    bool print_even = false;    

    // if a file is passed to the program it will be the final argument
    input = fopen(argv[argc - 1], "r");
    if (input == NULL) { // if unable to open file, file was not passed and should read from stdin
        input = stdin;
    }

    // opt is an int to account for all values that can be returned by getopt
    // getopt can return -1 when all options processed, : when option missing argument, etc.
    // these cannot be handled by char as values may lie outside normal range of ASCII characters
    int opt;
    
    while((opt = getopt(argc, argv, "hVen:")) != -1) {
        switch(opt) {
            case 'h':
                // display all options
                print_usage(prog_name); 
                return 0;
                break;
            case 'V':
                // output version info
                print_version_info();
                return 0;
                break;
            case 'e':
                // print even lines
                print_even = true;
                break;
            case 'n': {
                // option n comes with argument
                int arg = atoi(optarg);
                if (arg >= 0) {
                    num_lines = arg;
                }
                break;
            }
            default:
                printf("Unhandled argument caught");
                break;
        }
    }
    head(input, num_lines, print_even);
    return 0; 
}


