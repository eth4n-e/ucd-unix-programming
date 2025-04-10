#ifndef HELPERS_H
#define HELPERS_H
#include <stddef.h>

/*
Name: Ethan Epperson
Student Number: 25201495
Email: ethan.epperson@ucdconnect.ie
*/

void display_prompt();
struct tm* get_current_time();
char** parse_and_store_tokens(char *line);
void execute_chdir(char **args);
static size_t count_strings(char **arr);

#endif 
