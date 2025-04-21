#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>

void move_cursor_left();

void move_cursor_right();

// Initializes the terminal for raw mode
void enable_raw_mode();

// Restores the terminal to its original settings
void disable_raw_mode();

#endif
