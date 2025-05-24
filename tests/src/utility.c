#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <utility.h>

const char *mock_stdout_filepath = "/tmp/mock_stdout";

void read_file_to_buffer(FILE *output, char* buffer, size_t length) {
    // Rewind the temporary file and read its contents to `buffer`
    rewind(output);
    char* result = fgets(buffer, length, output);

    if (!result) {
        fprintf(stderr, "%s\n", "Error while reading stdout into buffer (possibly empty contents)");
        exit(1);
    }
}

FILE* write_to_mock_stdin(const char* input) {
    FILE *temp_file = tmpfile();
    if (!temp_file) {
        perror("tmpfile");
        exit(1);
    }

    // Write test input to the temporary file
    fputs(input, temp_file);
    rewind(temp_file);  // Reset file pointer to the beginning

    // Redirect stdin to the temporary file
    stdin = temp_file;

    return temp_file;
}

FILE *get_mock_stdout_file() {
    FILE *mock_stdout_file = fopen(mock_stdout_filepath, "w+");
    if (mock_stdout_file == NULL) {
        log_error_with_exit("Failed to open/create mock file");
    }

    return mock_stdout_file;
}

FILE *write_to_mock_stdout(char *input) {
    FILE *mock_stdout_file = get_mock_stdout_file();
    fprintf(mock_stdout_file, "%s", input);

    return mock_stdout_file;
}
