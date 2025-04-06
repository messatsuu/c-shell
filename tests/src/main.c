#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>

extern int run_suite_test_ps1();
extern int run_suite_test_command();

int main(void) {
    unsigned int result = 0;

    result |= run_suite_test_ps1();
    // result |= run_suite_test_command();

    return 0;
}
