#ifndef UTILITY_TEST_H
#define UTILITY_TEST_H

#include <stdio.h>

// Redirect stdout to a temporary file
FILE *mock_stdout();
// Rewind the temporary file and read its contents to `buffer`
void read_file_to_buffer(FILE *output, char* buffer, size_t length);
// Put `input` into a temporary file and use it as stdin
FILE* mock_stdin(const char* input);

#endif
