#ifndef SHELL_H
#define SHELL_H

// Reset terminal input buffer (used in SIGINT handling)
void reset_shell();

// Main shell loop
void create_prompt();

void execute_input();

#endif
