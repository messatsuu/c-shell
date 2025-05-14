#include "../include/input.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

static struct termios orig_termios;

void move_cursor_left() {
    printf("\x1b[D");
    fflush(stdout);
}

void move_cursor_right() {
    printf("\x1b[C");
    fflush(stdout);
}

void move_cursor_left_word(InputBuffer *inputBuffer) {
    unsigned int initial_cursor_position = inputBuffer->cursor_position;

    for (const char *pointer = &inputBuffer->buffer[inputBuffer->cursor_position]; inputBuffer->cursor_position != 0; pointer--) {
        // Only break if the first character is not already a space
        if (*pointer == ' ' && inputBuffer->cursor_position != initial_cursor_position) {
            break;
        }

        inputBuffer->cursor_position--;
        printf("\x1b[D");
    }

    fflush(stdout);
}

void move_cursor_right_word(InputBuffer *inputBuffer) {
    inputBuffer->buffer[inputBuffer->length] = '\0';
    unsigned int initial_cursor_position = inputBuffer->cursor_position;

    for (const char *pointer = &inputBuffer->buffer[inputBuffer->cursor_position]; *pointer != '\0'; pointer++) {
        // Only break if the first character is not already a space
        if (*pointer == ' ' && inputBuffer->cursor_position != initial_cursor_position) {
            break;
        }

        inputBuffer->cursor_position++;
        printf("\x1b[C");
    }

    fflush(stdout);
}

void move_cursor_to_start(InputBuffer *inputBuffer) {
    inputBuffer->cursor_position = 0;
}

void move_cursor_to_end(InputBuffer *inputBuffer) {
    inputBuffer->cursor_position = inputBuffer->length;
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN); // Disable echo and canonical mode
    raw.c_iflag &= ~(IXON); // Disable XON/XOFF flow control
    raw.c_oflag &= ~OPOST;  // Disable output processing
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
