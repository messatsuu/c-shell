#ifndef TERMINAL_H
#define TERMINAL_H

#include "../include/input.h"
#include <termios.h>

void move_cursor_left();

void move_cursor_right();

void move_cursor_left_word(InputBuffer *inputBuffer);

void move_cursor_right_word(InputBuffer *inputBuffer);

void move_cursor_to_start(InputBuffer *inputBuffer);

void move_cursor_to_end(InputBuffer *inputBuffer);

void delete_cursor_left_word(InputBuffer *inputBuffer);

// Initializes the terminal for raw mode
void enable_raw_mode();

// Restores the terminal to its original settings
void disable_raw_mode();

#endif
