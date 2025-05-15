#include <process.h>
#include <shell.h>
#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <utility>

static void test_echo_output(void **state) {
    // Setup
    FILE *stdin_mock = mock_stdin("echo foo bar\n");
    // Since we use a pipe and print to stdout in the main process (instead of child process)
    // we can use the stdout mock directly
    FILE *stdout_mock = mock_stdout();

    char buffer[1024];

    // Run
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

int run_suite_test_command() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_echo_output),
    };

    return cmocka_run_group_tests(tests, setup, teardown);
}
