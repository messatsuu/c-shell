#include "../include/utility.h"
#include "core/process.h"
#include "core/shell.h"

#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <utility.h>

static void test_echo_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    write_to_mock_stdin("echo foo bar\n");
    execute_input();

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "foo bar\n");
}

static int setup(void **state) {
    return 0;
}

static int teardown(void **state) {
    return 0;
}

unsigned int run_suite_test_command() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_echo_output),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
