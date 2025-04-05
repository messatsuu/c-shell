#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>

#include "../../include/shell.h"
#include <stdlib.h>
#include <string.h>
#include "../include/utility.h"

// Mock implementation of `gethostname`
ssize_t gethostname(char *name, size_t len) {
    strcpy(name, "test-host");
    return 0;
}

static void test_ps1_output(void **state) {
    // Setup
    FILE *stdout_mock = mock_stdout();
    setenv("USER", "test-user", 1);
    setenv("PWD", "/usr/bin/c-shell-implementation", 1);
    setenv("PS1", "[\\u@\\w on \\h] $", 1);

    // Run
    create_ps1();

    // Assert
    char buffer[100];
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "[test-user@/usr/bin/c-shell-implementation on test-host] $");
}

static int setup(void **state) {
    return 0;
}

static int teardown(void **state) {
    return 0;
}

int run_suite_test_ps1() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ps1_output),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
