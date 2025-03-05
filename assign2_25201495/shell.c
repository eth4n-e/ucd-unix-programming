#include <stdio.h>
#include <stdlib.h>

/* Notes
- Define one header file for useful functions, perhaps signal handlers, error handling, etc.
- use one c source file to implement these function definitions
- the other c source file should be for the main program
*/

static char* prompt = "#";

int main (int argc, char **argv) {
    FILE *input = stdin;
    // to store inputted line
    char *line = NULL;
    // default buffer allocation
    size_t len = 0;
    // number characters read
    ssize_t read;

    printf("%s ", prompt);
    while ((read = getline(&line, &len, input)) != -1) {
        printf("%s ", prompt);
    }
}
