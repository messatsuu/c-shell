#include "core/shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <utility.h>

// Make sure that `prompt` is initialized as "", since the function is called multiple times (may lead to undefined behavior)
char prompt[1000] = "";

char *create_ps1() {
    char *ps1 = getenv("PS1");
    memset(prompt, 0, 100);

    // If PS1 is not set, we use a default prompt
    if (ps1 == NULL) {
        ps1 = "\e[0;34mc-shell \\u@\\h> \e[m";
    }

    size_t index = 0;

    // Example input: \n\[\033[1;32m\][\u:\w]\$\[\033[0m\]
    // Example output: \033[1;32m[user:current-dir]$\033[0m
    for (const char *pointer = ps1; *pointer != '\0'; pointer++) {
        // if the character isn't a '\', simply add it to ouput
        if (*pointer != '\\') {
            prompt[index++] = *pointer;
            continue;
        }

        // If the current character is '\', handle the char after it
        pointer++;

        // Skip bash escape sequences ('\]' & '\[') and newlines ('\n')
        if (*pointer == '[' || *pointer == ']' || *pointer == 'n') {
            continue;
        }

        if (*pointer == 'a') { // End of a prompt title, add to output
            prompt[index++] = '\a';
            continue;
        }

        if (*pointer == 'e' || strncmp(pointer, "033", 3) == 0) {
            prompt[index++] = '\033';
            if (*pointer == '0') {  // Skip "033"
                pointer += 2;
            } else if (strncmp(pointer, "e]0;", 4) == 0) { // Stort of a prompt title, add to output
                strcat(prompt, "]0;");
                pointer += 3;
                index += 3;
            }
            continue;
        }
        char special_field[256] = "";

        // Get the env variable and copy its contents to `special_field` (overrides existing contents with strncpy);
        switch (*pointer) {
            case 'u': {
                set_env_field(special_field, sizeof(special_field), "USER");
                break;
            }
            case 'w': {
                set_env_field(special_field, sizeof(special_field), "PWD");
                break;
            }
            case 'h': {
                get_host_name(special_field, sizeof(special_field));
                break;
            }
            default:
                // Unknown sequence, keep it as is
                prompt[index++] = *pointer;
                continue;
        }

        // Concat the contents of `special_field` to prompt and move index forward
        strncat(prompt, special_field, strlen(special_field));
        index += strlen(special_field);
    }

    return prompt;
}

