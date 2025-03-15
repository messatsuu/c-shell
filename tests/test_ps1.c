#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>

#include "../include/shell.h"
#include <stdlib.h>
#include <string.h>

FILE *stdout_mock = NULL;

void redirect_stdin() {
    const char *test_input = "echo this is a test";
    FILE *memstream = fmemopen((void *)test_input, strlen(test_input), "r");

    // Save the original stdin, and redirect stdin to the memory stream
    FILE *original_stdin = stdin;
    stdin = memstream;
}

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

void read_stdout_to_buffer(FILE *output, char* buffer, size_t length) {
    // Rewind the temporary file and read its contents to `buffer`
    rewind(output);
    char* result = fgets(buffer, length, output);
    if (!result) {
        perror("Error while reading stdout into buffer");
    }
}

// Mock implementation of `gethostname`
ssize_t gethostname(char *name, size_t len) {
    strcpy(name, "test-host");
    return 0;
}

static void test_ps1_output(void **state) {
    // Setup
    setenv("USER", "test-user", 1);
    setenv("PWD", "c-shell-implementation", 1);
    setenv("PS1", "[\\u@\\w on \\h] $", 1);

    // Run
    create_ps1();

    // Assert
    char buffer[100];
    read_stdout_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "[test-user@c-shell-implementation on test-host] $");
}

// Setup and teardown (if needed for setup/cleanup before/after tests)
static int setup(void **state) {
    stdout_mock = mock_stdout();
    return 0;
}

static int teardown(void **state) {
    return 0;
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ps1_output),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
