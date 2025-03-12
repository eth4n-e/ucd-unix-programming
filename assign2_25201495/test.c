#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char input_str[] = "One two three four";
    char **output_str = malloc(sizeof(char*));
    int count = 0;
    char *temp = strtok(input_str, " ");
    while (temp != NULL) {
        output_str[count] = temp;
        count++;
        output_str = realloc(output_str, (count + 1) * sizeof(char*));
        temp = strtok(NULL, " ");
    }
    
    int iter = 0;
    while(output_str[iter] != NULL) {
        printf("%d, output_str[%d]: %s", iter, iter, output_str[iter]);
        iter++;
    }     

    return 0;
}
