#include "core/terminal.h"
#include "utility.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static struct termios orig_termios;

void print_under_input(const char *message) {
    printf("\r");           // Go to start of current line
    printf("\x1b[2K");      // Clear entire line
    printf("\n%s\n", message);
    fflush(stdout);
}

void move_cursor_left() {
    printf("\x1b[D");
    fflush(stdout);
}

void move_cursor_right() {
    printf("\x1b[C");
    fflush(stdout);
}

void move_cursor_left_word(InputBuffer *inputBuffer) {
    unsigned int new_cursor_position = 0;
    if (inputBuffer->cursor_position > 0) {
        for (int i = inputBuffer->cursor_position - 1; i >= 0; i--) {
            if (inputBuffer->buffer[i] == ' ') {
                new_cursor_position = i;
                break;
            }
        }
    }
    inputBuffer->cursor_position = new_cursor_position;
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

void delete_cursor_left_word(InputBuffer *inputBuffer) {
    unsigned int initial_cursor_position = inputBuffer->cursor_position;

    // Decrease cursor_position to next space
    for (const char *pointer = &inputBuffer->buffer[inputBuffer->cursor_position - 1]; inputBuffer->cursor_position != 0; pointer--) {
        // Only break if the first character is not already a space
        if (*pointer == ' ' && inputBuffer->cursor_position != initial_cursor_position) {
            break;
        }

        inputBuffer->cursor_position--;
    }

    // Move memory from the initial cursor position to the current one
    memmove(&inputBuffer->buffer[inputBuffer->cursor_position], &inputBuffer->buffer[initial_cursor_position], inputBuffer->length - initial_cursor_position);
    inputBuffer->length -= initial_cursor_position - inputBuffer->cursor_position;
    inputBuffer->buffer[inputBuffer->length] = '\0';
}

void delete_cursor_to_start(InputBuffer *inputBuffer) {
    memmove(&inputBuffer->buffer[0], &inputBuffer->buffer[inputBuffer->cursor_position], inputBuffer->length - inputBuffer->cursor_position);
    inputBuffer->length -= inputBuffer->cursor_position;
    inputBuffer->buffer[inputBuffer->length] = '\0';

    move_cursor_to_start(inputBuffer);
}

void delete_cursor_to_end(InputBuffer *inputBuffer) {
    inputBuffer->buffer[inputBuffer->cursor_position] = '\0';
    inputBuffer->length = inputBuffer->cursor_position;
}

int get_terminal_columns_count() {
    struct winsize win_size;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win_size) == -1) {
        log_error("Failed to get terminal columns");
        return -1;
    }

    return win_size.ws_col;
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

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

