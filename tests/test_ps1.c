#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>

#include "../include/shell.h"
#include <stdlib.h>
#include <string.h>

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

void put_stdout_to_buffer(FILE *output, char* buffer, size_t length) {
    // Rewind the temporary file and read its contents to `buffer`
    rewind(output);
    fgets(buffer, length, output);
}

// Mock implementation of `gethostname`
ssize_t gethostname(char *name, size_t len) {
    strcpy(name, "test-host");
    return 0;
}

static void test_shell(void **state) {
    // Setup
    FILE *stdout_mock = mock_stdout();
    setenv("USER", "test-user", 1);
    setenv("PWD", "c-shell-implementation", 1);
    setenv("PS1", "[\\u@\\w on \\h] $", 1);

    // Run
    create_ps1();

    // Assert
    char buffer[100];
    put_stdout_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "[test-user@c-shell-implementation on test-host] $");
}

// Setup and teardown (if needed for setup/cleanup before/after tests)
static int setup(void **state) {
    (void) state;
    return 0;  // success
}

static int teardown(void **state) {
    (void) state;
    return 0;  // success
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_shell),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
