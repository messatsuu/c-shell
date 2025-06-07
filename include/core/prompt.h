#ifndef PROMPT_H
#define PROMPT_H

extern char prompt[100];

// create the PS1 Prompt
char *create_ps1();

// Get the prompt's visible length
unsigned int get_prompt_visible_length();

#endif
