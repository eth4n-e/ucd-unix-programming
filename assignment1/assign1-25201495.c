#include <ctype.h>
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
- Program implements head functionality with both even and odd as options
- Prints the number of specified lines from a provided file (default: 10)
- Prints even lines within a file if specified
- Prints odd lines within a file if specified
- If both even or odd passed, the second option takes precedence
- If no file is provided, user is prompted to input to stdin where 
- the number of lines specified will be printed

SUCCESS OF PROGRAM
- The program successfully implements the commands -n, -V, -h, -o, and -e 
- I believe the program works completely but in terms of implementing reading from stdin
- I am not sure if I was meant to implement it such that if no file is provided on the command line
- that the user could input text that would be printed
- my implementation prints a message indicating to the user that the program is waiting for input from 
- stdin to which the user can begin entering text. This of course takes place while the program is running
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
    printf("Usage: %s head [options] ... [file]\n", program_name);
    printf("Options:\n");
    printf("    -n K        Output first K lines\n"); 
    printf("    -V          Output version info\n");
    printf("    -h          Display usage\n");
    printf("    -e          Print even lines\n");
    printf("    -o          Print odd lines\n");
    return;
}

void head(FILE *input, int num_lines, bool *even) {
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
        printf("Input lines to stdin: \n");
    }
 
    while (count < num_lines && (read = getline(&line, &len, input)) != -1) {
        // check if even flag has been set first, if it is null we store lines normally
        // if even flag set and on an even line, store
        // if odd flag set and on odd line, store
        if (even == NULL ||
            (*even && (curr_lin % 2) == 0) ||
            (!*even && (curr_lin % 2) == 1)) {  
            // essentially we want to store the line and not the mem address of the line
            // without duplicating, lines[*] would all point to mem address of line
            // and line is overwritten each time
            lines[count] = strdup(line);
            if (!lines[count]) {
                perror("Memory allocation failed");
                break;      
            }
            count++;
        }
        
        // move to next line
        curr_lin += 1;
    }
     
    if (input == stdin) {
        printf("\nOutput: \n");
    }
    
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

    const char *prog_purpose = "head";
    char *secondArg = argv[1];
    // convert arg we will compare to lower case
    // allows for more variability in input
    // e.g ./prog_name hEaD or /prog_name HEAD
    for (int i = 0; secondArg[i]; i++) {
        secondArg[i] = tolower(secondArg[i]);
    }

    if (strcmp(prog_purpose, secondArg) != 0) {
        printf("Second argument to program must be a variant of head\n");
        return 0;
    }     

    FILE *input;
    // defaults for head program
    int num_lines = 10;
    // use bool pointer to account for 3 values: NULL, true, false
    // if even / odd passed, can use a var to store value and update bool pointer
    bool *print_even = NULL;    
    bool print_even_val;
    // if a file is passed to the program it will be the final argument
    input = fopen(argv[argc - 1], "r");
    if (input == NULL) { // if unable to open file, file was not passed and should read from stdin
        input = stdin;
    }


    // move argv forward, by default getopt processes argv[1] but this must be "head"
    // arguments following will be options for the program that need to be processed
    // decrement argc because we have one less arg after accounting for "head"
    argv += 1;
    argc -= 1;
    // opt is an int to account for all values that can be returned by getopt
    // getopt can return -1 when all options processed, : when option missing argument, etc.
    // these cannot be handled by char as values may lie outside normal range of ASCII characters
    int opt;
    
    while((opt = getopt(argc, argv, "hVeon:")) != -1) {
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
                print_even_val = true;
                print_even = &print_even_val;
                break;
            case 'o':
                // print odd lines
                print_even_val = false;
                print_even = &print_even_val;
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


