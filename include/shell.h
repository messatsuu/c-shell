#ifndef SHELL_H
#define SHELL_H

#include <string.h>

// Reset terminal input buffer (used in SIGINT handling)
void reset_shell();

// Reads input from the user
char *read_input();

// Convert env-vars to their value
char *convert_input(const char *input);

// Main shell loop
void create_prompt();

void set_env_field(char *special_field, size_t special_field_size, char *env_var);

void execute_input();

// create the PS1 Prompt
void create_ps1();

#endif
