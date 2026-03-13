#ifndef SHELL_H
#define SHELL_H

// Reset terminal input buffer (used in SIGINT handling)
void reset_shell();

char *get_user_input();

// Main shell loop
void execute_input(char *original_input);

#endif
