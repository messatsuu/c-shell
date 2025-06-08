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
    unsigned int old_buffer_size = inputBuffer->buffer_size;
    inputBuffer->buffer_size += buffer_expansion_size;

    inputBuffer->buffer = reallocate_safe(inputBuffer->buffer, old_buffer_size, inputBuffer->buffer_size, false);
    inputBuffer->buffer_backup = reallocate_safe(inputBuffer->buffer_backup, old_buffer_size, inputBuffer->buffer_size, false);

    if (!inputBuffer->buffer || !inputBuffer->buffer_backup) {
        log_error("Reallocation of input buffer failed");
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

    printf("\r");

    // Move to correct cursor position, only visible part of prompt
    printf("\033[%dC", get_prompt_visible_length() + inputBuffer->cursor_position);
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
            if (reallocate_input_buffer(inputBuffer, history_entry_length) == -1) {
                return;
            };
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

        if (inputBuffer.length + 1 >= inputBuffer.buffer_size) {
            if (reallocate_input_buffer(&inputBuffer, BUF_EXPANSION_SIZE) == -1) {
                return NULL;
            }
        }

        switch (current_char) {
            case 4: // Ctrl+D
                cleanup_input_buffer(&inputBuffer);
                return NULL;

            case 10: // Enter
                inputBuffer.buffer[inputBuffer.length] = '\0';
                goto done;

            case 127: // Backspace (DEL)
            case 8:   // Backspace (BS)
                if (inputBuffer.cursor_position > 0) {
                    memmove(&inputBuffer.buffer[inputBuffer.cursor_position - 1],
                            &inputBuffer.buffer[inputBuffer.cursor_position],
                            inputBuffer.length - inputBuffer.cursor_position);
                    inputBuffer.cursor_position--;
                    inputBuffer.length--;
                    inputBuffer.buffer[inputBuffer.length] = '\0';
                }
                break;

            case 27: { // Escape sequence
                int next_char = getchar();
                if (next_char != '[' && next_char != ';') break;

                next_char = getchar();
                switch (next_char) {
                    case 'A': // Up
                        set_history_entry_to_buffer(+1, &inputBuffer);
                        break;
                    case 'B': // Down
                        set_history_entry_to_buffer(-1, &inputBuffer);
                        break;
                    case 'C': // Right
                        if (inputBuffer.cursor_position < inputBuffer.length) {
                            inputBuffer.cursor_position++;
                            move_cursor_right();
                        }
                        break;
                    case 'D': // Left
                        if (inputBuffer.cursor_position > 0) {
                            inputBuffer.cursor_position--;
                            move_cursor_left();
                        }
                        break;
                    case '1': {
                        if (getchar() != ';' || getchar() != '5') {
                            break;
                        }
                        next_char = getchar();
                        switch (next_char) {
                            case 'C': // Ctrl+Right
                                move_cursor_right_word(&inputBuffer);
                                break;
                            case 'D': // Ctrl+Left
                                move_cursor_left_word(&inputBuffer);
                                break;
                        }
                        break;
                    }
                }
                break;
            }

            case 1: // Ctrl+A
                move_cursor_to_start(&inputBuffer);
                break;
            case 5: // Ctrl+E
                move_cursor_to_end(&inputBuffer);
                break;
            case 12: // Ctrl+L
                printf("\e[1;1H\e[2J");
                break;
            case 21: // Ctrl+U
                delete_cursor_to_start(&inputBuffer);
                break;
            case 11: // Ctrl+K
                delete_cursor_to_end(&inputBuffer);
                break;
            case 23: // Ctrl+W
                delete_cursor_left_word(&inputBuffer);
                break;
            case 9: // Tab
                autocomplete(&inputBuffer);
                break;
            default:
                if (current_char >= 32 && current_char <= 126) {
                    char char_string[2] = { (char)current_char, '\0' };
                    insert_into_buffer_at_cursor_position(&inputBuffer, char_string, 1);
                }
                break;
        }

        redraw_line(&inputBuffer);
    }

done:
    disable_raw_mode();
    printf("\n");

    char *input = allocate(inputBuffer.length + 1, false);
    if (input != NULL) {
        memcpy(input, inputBuffer.buffer, inputBuffer.length);
        input[inputBuffer.length] = '\0';
    }

    cleanup_input_buffer(&inputBuffer);
    return input;
}
