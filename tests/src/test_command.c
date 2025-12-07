#include "../include/utility.h"
#include "core/shell.h"

#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <utility.h>

static void test_echo_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("echo -n foo; echo -n bar; echo baz\n"));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "foobarbaz\n");
}

static void test_piped_command_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("echo \"foo bar\" | tr ' ' '\\n' | sort\n"));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "bar\nfoo\n");
}

static void test_chained_command_output(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("false && echo AND || echo OR ; echo SEMI\n"));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "OR\nSEMI\n");
}

static void test_quote_handling(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    // Input: echo \" ; echo "''"
    execute_input(strdup("echo \\\\\" ; echo \"''\""));

    // Assert
    // Output: "\n''\n
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "\\\"\n''\n");
}

static void test_argument_handling(void **state) {
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("printf '[%s] ' 'first argument' second 'third argument'"));
    execute_input(strdup("printf '[%s] ' \"fourth argument :)\" fifth \"sixth argument\""));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "[first argument] [second] [third argument] [fourth argument :)] [fifth] [sixth argument] ");
}

static void test_output_redirection(void **state) {
    // Setup
    char *file_path = strdup("/tmp/redirect_output");
    char buffer[1024];

    // Run
    char *command = callocate(INITIAL_BUFSIZE_BIG, 1, true);
    sprintf(command, "echo we redirect > %s", file_path);
    execute_input(command);

    // Assert
    FILE *redirect_file = fopen(file_path, "r");
    read_file_to_buffer(redirect_file, buffer, sizeof(buffer));
    assert_string_equal(buffer, "we redirect\n");

    // cleanup
    fclose(redirect_file);
    free(file_path);
}

static void test_output_redirection_appending(void **state) {
    // Setup
    char *file_path = strdup("/tmp/redirect_output");
    char buffer[1024];

    // Run
    char *command = callocate(INITIAL_BUFSIZE_BIG, 1, true);
    sprintf(command, "echo we redirect > %s && echo we append >> %s", file_path, file_path);
    execute_input(command);

    // Assert
    FILE *redirect_file = fopen(file_path, "r");
    read_file_to_buffer(redirect_file, buffer, sizeof(buffer));
    assert_string_equal(buffer, "we redirect\nwe append\n");

    // cleanup
    fclose(redirect_file);
    free(file_path);
}

static void test_exit_code_priority_handling(void **state) {
    // TODO: fix this test, output doesn't redirect to stdout_mock since it isn't a child-process
    // Setup
    FILE *stdout_mock = get_mock_stdout_file();
    char buffer[1024];

    // Run
    execute_input(strdup("false | true ; last_exit_code"));

    // Assert
    read_file_to_buffer(stdout_mock, buffer, sizeof(buffer));
    assert_string_equal(buffer, "0");
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
        cmocka_unit_test_setup_teardown(test_quote_handling, setup, teardown),
        cmocka_unit_test_setup_teardown(test_argument_handling, setup, teardown),
        cmocka_unit_test_setup_teardown(test_output_redirection, setup, teardown),
        cmocka_unit_test_setup_teardown(test_output_redirection_appending, setup, teardown),
        // cmocka_unit_test_setup_teardown(test_exit_code_priority_handling, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
