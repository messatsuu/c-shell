#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility.h>

const char *mock_stdout_filepath = "/tmp/mock_stdout";

void read_file_to_buffer(FILE *output, char *buffer, size_t length) {
    if (fseek(output, 0, SEEK_SET) != 0) {
        perror("rewind failed");
        exit(1);
    }

    size_t total = 0;
    while (fgets(buffer + total, length - total, output) != NULL) {
        total = strlen(buffer);
        if (total >= length - 1) break;
    }

    if (total == 0) {
        fprintf(stderr, "Error while reading file into buffer (possibly empty contents)\n");
        exit(1);
    }
}

FILE *get_mock_stdout_file() {
    FILE *mock_stdout_file = fopen(mock_stdout_filepath, "a+");
    if (mock_stdout_file == NULL) {
        log_error_with_exit("Failed to open/create mock file");
    }

    return mock_stdout_file;
}

FILE *clear_mock_stdout_file() {
    FILE *mock_stdout_file = fopen(mock_stdout_filepath, "w");
    return mock_stdout_file;
}

FILE *write_to_mock_stdout(char *input) {
    FILE *mock_stdout_file = get_mock_stdout_file();
    fprintf(mock_stdout_file, "%s", input);

    return mock_stdout_file;
}
