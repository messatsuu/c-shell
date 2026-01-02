#include "../include/utility.h"
#include "core/shell.h"
#include "tests/include/utility.h"

#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <utility.h>

static void test_alias_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("alias my_alias='echo works!'"));
    execute_input(strdup("my_alias"));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "works!\n");
}

static void test_recursive_alias_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("alias my_alias='echo works!' another_alias=my_alias"));
    execute_input(strdup("another_alias"));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "works!\n");
}

static void test_infinite_recursive_alias_stops_expansion(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("alias foo=bar bar=foo"));
    execute_input(strdup("foo ; echo hehe"));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    // TODO: fix this once we can get output from non fork-n-exec'd child
    assert_string_equal(buffer, "hehe\n");
}

static void test_unalias(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("alias my_alias='echo works!'"));
    execute_input(strdup("my_alias"));
    execute_input(strdup("unalias my_alias"));
    execute_input(strdup("my_alias"));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "works!\n");
}

static void test_unalias_all(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("alias my_alias='echo works!' another_alias='echo works!'"));
    execute_input(strdup("my_alias"));
    execute_input(strdup("unalias -a"));
    execute_input(strdup("my_alias"));
    execute_input(strdup("another_alias"));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "works!\n");
}

static int setup(void **state) {
    return 0;
}

static int teardown(void **state) {
    clear_mock_stdout_file();
    return 0;
}

unsigned int run_suite_test_alias() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_alias_output, setup, teardown),
        cmocka_unit_test_setup_teardown(test_recursive_alias_output, setup, teardown),
        cmocka_unit_test_setup_teardown(test_infinite_recursive_alias_stops_expansion, setup, teardown),
        cmocka_unit_test_setup_teardown(test_unalias, setup, teardown),
        cmocka_unit_test_setup_teardown(test_unalias_all, setup, teardown),
        // TODO: add more test cases when making non fork-n-exec'd output capturable
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
