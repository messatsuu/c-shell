#ifndef TERMINAL_H
#define TERMINAL_H

#include "input/input.h"

#include <termios.h>

void print_under_input(const char *message);

void move_cursor_left();

void move_cursor_right();

void move_cursor_left_word(InputBuffer *inputBuffer);

void move_cursor_right_word(InputBuffer *inputBuffer);

void move_cursor_to_start(InputBuffer *inputBuffer);

void move_cursor_to_end(InputBuffer *inputBuffer);

void delete_cursor_left_word(InputBuffer *inputBuffer);

void delete_cursor_to_start(InputBuffer *inputBuffer);

void delete_cursor_to_end(InputBuffer *inputBuffer);

int get_terminal_columns_count();

// Initializes the terminal for raw mode
void enable_raw_mode();

// Restores the terminal to its original settings
void disable_raw_mode();

#endif
