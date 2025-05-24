#include "../include/utility.h"
#include "core/shell.h"

#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <utility.h>

static void test_echo_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    write_to_mock_stdin("echo -n foo; echo -n bar; echo baz\n");
    execute_input();

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "foobarbaz\n");
}

static void test_piped_command_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    write_to_mock_stdin("ls / | grep bin\n");
    execute_input();

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "bin\n");
}

static void test_chained_command_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    write_to_mock_stdin("false && echo AND || echo OR ; echo SEMI\n");
    execute_input();

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "OR\nSEMI\n");
}

static int setup(void **state) {
    return 0;
}

static int teardown(void **state) {
    clear_mock_stdout_file();
    return 0;
}

unsigned int run_suite_test_command() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_echo_output, setup, teardown),
        cmocka_unit_test_setup_teardown(test_piped_command_output, setup, teardown),
        cmocka_unit_test_setup_teardown(test_chained_command_output, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
