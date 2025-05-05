#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/utility.h"
#include "../include/history.h"
#include "../include/prompt.h"
#include "../include/terminal.h"
#include <signal.h>
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
extern History *history;

typedef struct {
    char *buffer;
    char *buffer_backup;
    unsigned int length;
    unsigned int cursor_position;
    unsigned int historyIndex;
    unsigned int buffer_size;
} InputBuffer;

void init_input_buffer(InputBuffer *inputBuffer) {
    inputBuffer->buffer = allocate(INITIAL_BUFSIZE, true);
    inputBuffer->buffer_backup = allocate(INITIAL_BUFSIZE, true);
    inputBuffer->length = 0;
    inputBuffer->cursor_position = 0;
    inputBuffer->historyIndex = 0;
    inputBuffer->buffer_size = INITIAL_BUFSIZE;
    memset(inputBuffer->buffer, 0, INITIAL_BUFSIZE);
    memset(inputBuffer->buffer_backup, 0, INITIAL_BUFSIZE);
}

void cleanup_input_buffer(InputBuffer *inputBuffer) {
    if (inputBuffer == NULL) {
        return;
    }
    free(inputBuffer->buffer);
    free(inputBuffer->buffer_backup);
}

void redraw_line(InputBuffer *inputBuffer) {
    printf("\r");                 // go to beginning of line
    printf("%s%s", prompt, inputBuffer->buffer);       // reprint input
    printf("\x1b[K");             // clear from cursor to end

    unsigned int move_left = inputBuffer->length - inputBuffer->cursor_position;

    for (int i = 0; i < move_left; i++) {
        move_cursor_left();      // restore cursor position
    }
    fflush(stdout);
}

void set_history_entry_to_buffer(
    unsigned int incrementValue,
    InputBuffer *inputBuffer
) {
    if (history == NULL) {
        return;
    }
    
    // Convert history's count to a signed int to avoid wrapping issues
    int newHistoryIndex = (int)history->count - (int)(inputBuffer->historyIndex + incrementValue);
    // If the new history index is out of bounds, return
    if (newHistoryIndex < 0 || newHistoryIndex > history->count) {
        return;
    }

    if (inputBuffer->historyIndex == 0) {
        // Save the current buffer to the backup buffer
        strncpy(inputBuffer->buffer_backup, inputBuffer->buffer, inputBuffer->buffer_size);
    }

    // If the new history index is same as the count (outside of bounds), restore the backed up buffer
    if (newHistoryIndex == history->count) {
        strncpy(inputBuffer->buffer, inputBuffer->buffer_backup, inputBuffer->buffer_size);
    } else {
        // Otherwise we copy the history entry to the buffer
        strncpy(inputBuffer->buffer, history->entries[newHistoryIndex], inputBuffer->buffer_size);
    }

    inputBuffer->historyIndex = inputBuffer->historyIndex + incrementValue;
    inputBuffer->length = strlen(inputBuffer->buffer);
    inputBuffer->cursor_position = inputBuffer->length;
    redraw_line(inputBuffer);
}

char *read_input_prompt() {
    InputBuffer inputBuffer;
    init_input_buffer(&inputBuffer);

    enable_raw_mode();

    while (1) {
        int currentChar = getchar();

        // If SIG_INT is captured, clear the buffer and redraw
        if (sigint_received) {
            memset(inputBuffer.buffer, 0, inputBuffer.buffer_size);
            inputBuffer.length = 0;
            inputBuffer.cursor_position = 0;
            sigint_received = 0;
            redraw_line(&inputBuffer);
            continue;
        }

        if (currentChar == 4) { // Ctrl+C, Ctrl+D
            cleanup_input_buffer(&inputBuffer);
            return NULL;
        }

        if (currentChar == 10) { // Enter
            inputBuffer.buffer[inputBuffer.length] = '\0';
            break;
        }

        if (inputBuffer.length >= inputBuffer.buffer_size) {
            inputBuffer.buffer_size += BUF_EXPANSION_SIZE;
            inputBuffer.buffer = reallocate(inputBuffer.buffer, inputBuffer.buffer_size, false);
            inputBuffer.buffer_backup = reallocate(inputBuffer.buffer_backup, inputBuffer.buffer_size, false);

            if (!inputBuffer.buffer || !inputBuffer.buffer_backup) {
                return NULL;
            }
        }

        if (inputBuffer.cursor_position > 0 && (currentChar == 127 || currentChar == 8)) { // Backspace
            memmove(&inputBuffer.buffer[inputBuffer.cursor_position - 1], &inputBuffer.buffer[inputBuffer.cursor_position], inputBuffer.length - inputBuffer.cursor_position);
            inputBuffer.cursor_position--;
            inputBuffer.length--;
            inputBuffer.buffer[inputBuffer.length] = '\0';
            redraw_line(&inputBuffer);
        } else if (currentChar == 27) { // Escape sequence
            int nextChar = getchar();
            if (nextChar != '[' && nextChar != ';') {
                continue;
            }

            // TODO: implement CTRL-modifier (move by words)
            switch (getchar()) {
                case 'A': // up
                    set_history_entry_to_buffer(+1, &inputBuffer);
                    break;
                case 'B': // down
                    set_history_entry_to_buffer(-1, &inputBuffer);
                    break;
                case 'C': // right
                    if (inputBuffer.cursor_position < inputBuffer.length) {
                        inputBuffer.cursor_position++;
                        move_cursor_right();
                    }
                    break;
                case 'D': // left
                    if (inputBuffer.cursor_position > 0) {
                        inputBuffer.cursor_position--;
                        move_cursor_left();
                    }
                    break;
            }
        } else if (currentChar >= 32 && currentChar <= 126) { // Printable characters
            memmove(&inputBuffer.buffer[inputBuffer.cursor_position + 1], &inputBuffer.buffer[inputBuffer.cursor_position], inputBuffer.length - inputBuffer.cursor_position);
            inputBuffer.buffer[inputBuffer.cursor_position] = (char)currentChar;
            inputBuffer.cursor_position++;
            inputBuffer.length++;
            redraw_line(&inputBuffer);
        }
    }

    disable_raw_mode();
    printf("\n");

    char *input = allocate(inputBuffer.length + 1, false);
    if (input != NULL) {
        memcpy(input, inputBuffer.buffer, inputBuffer.length);
        input[inputBuffer.length] = '\0'; // Null-terminate manually
    }

    cleanup_input_buffer(&inputBuffer);
    return input;
}
