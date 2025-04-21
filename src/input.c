#include <signal.h>
#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/utility.h"
#include "../include/prompt.h"
#include "../include/terminal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_COMMAND_CAPACITY 1
#define INITIAL_BUFSIZE 20
#define BUF_EXPANSION_SIZE 100
#define MAX_ENV_VAR_NAME_BUFSIZE 128

// Separators
#define SEQUENTIAL_SEPARATOR = 0
#define AND_SEPARATOR = 1
#define OR_SEPARATOR = 2

volatile sig_atomic_t sigint_received = 0;

void redraw_line(char *buffer, unsigned int len, unsigned int cursor_position) {
    printf("\r");                 // go to beginning of line
    printf("%s%s", prompt, buffer);       // reprint input
    printf("\x1b[K");             // clear from cursor to end

    unsigned int move_left = len - cursor_position;

    for (int i = 0; i < move_left; i++) {
        move_cursor_left();      // restore cursor position
    }
    fflush(stdout);
}

char *read_input_prompt() {
    unsigned int buffer_size = INITIAL_BUFSIZE;
    unsigned int length = 0;
    unsigned int cursor = 0;

    char *buffer = allocate(buffer_size, true);
    memset(buffer, 0, buffer_size);

    enable_raw_mode();

    while (1) {
        int c = getchar();

        // If SIG_INT is captured, clear the buffer and redraw
        if (sigint_received) {
            memset(buffer, 0, buffer_size);
            length = 0;
            cursor = 0;
            sigint_received = 0;
            redraw_line(buffer, length, cursor);
            continue;
        }

        if (c == 4) { // Ctrl+C, Ctrl+D
            free(buffer);
            return NULL;
        }

        if (c == 10) { // Enter
            buffer[length] = '\0';
            break;
        }

        if (length >= buffer_size) {
            buffer_size += BUF_EXPANSION_SIZE;
            buffer = reallocate(buffer, buffer_size, false);
            if (!buffer) {
                return NULL;
            }
        }

        if (cursor > 0 && (c == 127 || c == 8)) { // Backspace
            memmove(&buffer[cursor - 1], &buffer[cursor], length - cursor);
            cursor--;
            length--;
            buffer[length] = '\0';
            redraw_line(buffer, length, cursor);
        } else if (c == 27 && getchar() == '[') { // Escape sequence
            switch (getchar()) {
                case 'D': // left
                    if (cursor > 0) {
                        cursor--;
                        move_cursor_left();
                    }
                    break;
                case 'C': // right
                    if (cursor < length) {
                        cursor++;
                        move_cursor_right();
                    }
                    break;
            }
        } else if (c >= 32 && c <= 126) { // Printable characters
            memmove(&buffer[cursor + 1], &buffer[cursor], length - cursor);
            buffer[cursor] = (char)c;
            cursor++;
            length++;
            redraw_line(buffer, length, cursor);
        }
    }

    disable_raw_mode();

    printf("\n");

    return buffer;
}
