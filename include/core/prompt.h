#ifndef PROMPT_H
#define PROMPT_H

extern char prompt[100];

// Parse $PS1 and return parsed string
char *get_prompt();

// Get the prompt's visible length
unsigned int get_prompt_visible_length();

#endif
