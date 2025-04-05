#include <stdio.h>
#include <stdlib.h>

FILE *mock_stdout() {
    FILE *temp_file = tmpfile();
    if (temp_file == NULL) {
        exit(0);
    }

    // Redirect stdout to the temporary file
    FILE *original_stdout = stdout;
    stdout = temp_file;

    return temp_file;
}

void read_file_to_buffer(FILE *output, char* buffer, size_t length) {
    // Rewind the temporary file and read its contents to `buffer`
    rewind(output);
    char* result = fgets(buffer, length, output);

    if (!result) {
        fprintf(stderr, "%s\n", "Error while reading stdout into buffer (possibly empty contents)");
        exit(1);
    }
}

FILE* mock_stdin(const char* input) {
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
