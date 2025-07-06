#include "command/command.h"
#include "command/builtins.h"
#include "core/process.h"
#include <utility.h>

#include <stdbool.h>
#include <unistd.h>

int last_exit_code = 0;

int execute_command(Command *command) {
    if (is_builtin_command(command->arguments[0])) {
        last_exit_code = run_builtin_command(command);
    } else {
        last_exit_code = run_child_process_piped(command);
    }

    return last_exit_code;
}
