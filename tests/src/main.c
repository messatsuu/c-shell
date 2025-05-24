#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>

extern unsigned int run_suite_test_ps1();
extern unsigned int run_suite_test_command();

int main(void) {
    unsigned int result = 0;

    unsigned int (*suites[])() = {
        run_suite_test_command,
        run_suite_test_ps1
    };
    const int number_of_suites = sizeof(suites) / sizeof(suites[0]);

    for (int i = 0; i < number_of_suites; i++) {
        result |= suites[i]();
        printf("\n");
    }

    return 0;
}
