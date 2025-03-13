#ifndef HELPERS_H
#define HELPERS_H

void display_prompt();
struct tm* get_current_time();
char** parse_and_store_tokens(char *line);
void execute_chdir(char **args);
static int count_strings(char **arr);

#endif 
