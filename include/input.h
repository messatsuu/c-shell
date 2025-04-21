#ifndef INPUT_H
#define INPUT_H

#include <signal.h>

extern volatile sig_atomic_t sigint_received;

// Reads input from the user
char *read_input_prompt();

#endif
