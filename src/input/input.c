#include "autocomplete/autocomplete.h"
#include "input/history.h"
#include "input/input.h"
#include "core/prompt.h"
#include "core/terminal.h"
#include <utility.h>

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_COMMAND_CAPACITY 1
#define MAX_ENV_VAR_NAME_BUFSIZE 128

// Separators
#define SEQUENTIAL_SEPARATOR = 0
#define AND_SEPARATOR = 1
#define OR_SEPARATOR = 2

volatile sig_atomic_t sigint_received = 0;
extern History *history;

void init_input_buffer(InputBuffer *inputBuffer) {
    inputBuffer->buffer = callocate(INITIAL_BUFSIZE, 1, true);
    inputBuffer->buffer_backup = callocate(INITIAL_BUFSIZE, 1, true);
    inputBuffer->length = 0;
    inputBuffer->cursor_position = 0;
    inputBuffer->history_index = 0;
    inputBuffer->buffer_size = INITIAL_BUFSIZE;
}

int reallocate_input_buffer(InputBuffer *inputBuffer, unsigned int buffer_expansion_size) {
    inputBuffer->buffer_size += buffer_expansion_size;
    inputBuffer->buffer = reallocate(inputBuffer->buffer, inputBuffer->buffer_size, false);
    inputBuffer->buffer_backup = reallocate(inputBuffer->buffer_backup, inputBuffer->buffer_size, false);

    if (!inputBuffer->buffer || !inputBuffer->buffer_backup) {
        return -1;
    }

    return 0;
}

void cleanup_input_buffer(InputBuffer *inputBuffer) {
    if (inputBuffer == NULL) {
        return;
    }
    free(inputBuffer->buffer);
    free(inputBuffer->buffer_backup);
}

void redraw_line(InputBuffer *inputBuffer) {
    // Null-terminate buffer for printf
    inputBuffer->buffer[inputBuffer->length] = '\0';

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
    int new_history_index = (int)history->count - (int)(inputBuffer->history_index + incrementValue);
    // If the new history index is out of bounds, return
    if (new_history_index < 0 || new_history_index > history->count) {
        return;
    }

    if (inputBuffer->history_index == 0) {
        // Save the current buffer to the backup buffer
        strncpy(inputBuffer->buffer_backup, inputBuffer->buffer, inputBuffer->buffer_size);
    }

    // If the new history index is same as the count (outside of bounds), restore the backed up buffer
    if (new_history_index == history->count) {
        strncpy(inputBuffer->buffer, inputBuffer->buffer_backup, inputBuffer->buffer_size);
    } else {
        // Otherwise we copy the history entry to the buffer and reallocate if needed
        int history_entry_length = strlen(history->entries[new_history_index]);
        if (history_entry_length + 1 >= inputBuffer->buffer_size) {
            reallocate_input_buffer(inputBuffer, history_entry_length);
        }
        strncpy(inputBuffer->buffer, history->entries[new_history_index], inputBuffer->buffer_size);
    }

    inputBuffer->history_index = inputBuffer->history_index + incrementValue;
    inputBuffer->length = strlen(inputBuffer->buffer);
    inputBuffer->cursor_position = inputBuffer->length;
    redraw_line(inputBuffer);
}

void insert_into_buffer_at_cursor_position(InputBuffer *inputBuffer, char *string, unsigned int string_length) {
    // Reallocate if needed
    if (inputBuffer->length + string_length + 1 >= inputBuffer->buffer_size) {
        if (reallocate_input_buffer(inputBuffer, BUF_EXPANSION_SIZE) == -1) {
            return;
        }
    }

    // Create space for the new string (Move string over `string_length` bytes)
    memmove(&inputBuffer->buffer[inputBuffer->cursor_position + string_length], &inputBuffer->buffer[inputBuffer->cursor_position], inputBuffer->length - inputBuffer->cursor_position);
    // Move the string into the new space
    memmove(&inputBuffer->buffer[inputBuffer->cursor_position], string, string_length);

    inputBuffer->length += string_length;
    inputBuffer->cursor_position += string_length;
}

char *read_input_prompt() {
    InputBuffer inputBuffer;
    init_input_buffer(&inputBuffer);

    enable_raw_mode();

    while (1) {
        int current_char = getchar();

        // If SIG_INT is captured, clear the buffer and redraw
        if (sigint_received) {
            memset(inputBuffer.buffer, 0, inputBuffer.buffer_size);
            inputBuffer.length = 0;
            inputBuffer.cursor_position = 0;
            sigint_received = 0;
            redraw_line(&inputBuffer);
            continue;
        }

        if (current_char == 4) { // Ctrl+C, Ctrl+D
            cleanup_input_buffer(&inputBuffer);
            return NULL;
        }

        if (current_char == 10) { // Enter
            inputBuffer.buffer[inputBuffer.length] = '\0';
            break;
        }

        if (inputBuffer.length + 1 >= inputBuffer.buffer_size) {
            if (reallocate_input_buffer(&inputBuffer, BUF_EXPANSION_SIZE) == -1) {
                return NULL;
            }
        }

        if (inputBuffer.cursor_position > 0 && (current_char == 127 || current_char == 8)) { // Backspace
            memmove(&inputBuffer.buffer[inputBuffer.cursor_position - 1], &inputBuffer.buffer[inputBuffer.cursor_position], inputBuffer.length - inputBuffer.cursor_position);
            inputBuffer.cursor_position--;
            inputBuffer.length--;
            inputBuffer.buffer[inputBuffer.length] = '\0';
        } else if (current_char == 27) { // Escape sequence
            int next_char = getchar();
            if (next_char != '[' && next_char != ';') {
                continue;
            }

            next_char = getchar();

            switch (next_char) {
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
                case '1':
                    next_char = getchar();
                    if (next_char != ';' || getchar() != '5') {
                        continue;
                    }
                    next_char = getchar();
                    switch (next_char) {
                        case 'C': // ctrl+right
                            move_cursor_right_word(&inputBuffer);
                            break;
                        case 'D': // ctrl+left
                            move_cursor_left_word(&inputBuffer);
                            break;
                    }
                    break;
            }
        } else if (current_char == 1) { // SOH (Start of Header), Ctrl+a
            move_cursor_to_start(&inputBuffer);
        } else if (current_char == 5) { // ENQ (Enquiry), Ctrl+e
            move_cursor_to_end(&inputBuffer);
        } else if (current_char == 12) { // FF (Form Feed), Ctrl+l
            // Print as newlines depedening on screen height
            printf("\e[1;1H\e[2J");
        } else if (current_char == 23) { // EOT (End of Transmission), Ctrl+w
            delete_cursor_left_word(&inputBuffer);
        } else if (current_char == 9) { // Tab
            autocomplete(&inputBuffer);
        } else if (current_char >= 32 && current_char <= 126) { // Printable characters
            // Convert Char to null-terminated string
            char char_string[2] = {(char)current_char, '\0'};
            insert_into_buffer_at_cursor_position(&inputBuffer, char_string, 1);
        }

        redraw_line(&inputBuffer);
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
