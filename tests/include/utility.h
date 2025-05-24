#ifndef UTILITY_TEST_H
#define UTILITY_TEST_H

#include <stdio.h>

// Rewind the temporary file and read its contents to `buffer`
void read_file_to_buffer(FILE *output, char* buffer, size_t length);
// Put `input` into a temporary file and use it as stdin
FILE* write_to_mock_stdin(const char* input);
// Get (and create) a mockfile to be used as stdout
FILE *get_mock_stdout_file();

FILE *write_to_mock_stdout(char *input);

FILE *clear_mock_stdout_file();

#endif
