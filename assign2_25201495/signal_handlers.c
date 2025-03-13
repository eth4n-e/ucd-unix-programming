#include <stdio.h>
#include <signal.h>
#include "signal_handlers.h"
#include "helpers.h"

/*
Name: Ethan Epperson
Student Number: 25201495
Email: ethan.epperson@ucdconnect.ie
*/

void handle_sigint(int sig) {
    // on Ctrl+c, ignore and continue prompting 
    // in the same way that bash shell does not exit following Ctrl+c
    printf("\n");
    display_prompt();
}

void setup_signal_handlers() {
   signal(SIGINT, handle_sigint); 
}
