#include "../include/utility.h"
#include "core/prompt.h"
#include "core/shell.h"
#include <utility.h>

#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>
#include <stdlib.h>

static void test_ps1_output(void **state) {
    // Setup
    set_environment_var("USER", "test-user", true);
    set_environment_var("PWD", "/usr/bin/c-shell-implementation", true);
    set_environment_var("PS1", "[\\u@\\w on \\h] $", true);

    // Run
    get_prompt();

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
