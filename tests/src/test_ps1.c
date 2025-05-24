#include "../include/utility.h"
#include <prompt.h>
#include <shell.h>
#include <utility.h>

#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>

// Mock implementation of `gethostname`
ssize_t gethostname(char *name, size_t len) {
    strcpy(name, "test-host");
    return 0;
}

static void test_ps1_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    setenv("USER", "test-user", 1);
    setenv("PWD", "/usr/bin/c-shell-implementation", 1);
    setenv("PS1", "[\\u@\\w on \\h] $", 1);

    // Run
    create_ps1();

    // Assert
    assert_string_equal(prompt, "[test-user@/usr/bin/c-shell-implementation on test-host] $");
}

static int setup(void **state) {
    return 0;
}

static int teardown(void **state) {
    return 0;
}

unsigned int run_suite_test_ps1() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ps1_output),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
