# C SHELL

Small shell project to learn more about C. Might actually make this halfway POSIX compliant.

## TODO
- [x] Read and interpret PS1 correctly
- [x] Read PS1 correctly
- [x] History tracking

- [ ] Capture CTRL+L to clear screen
- [ ] Traverse history with up-/down-arrow keys
- [ ] Implement pipes
- [ ] Use ncurses for editable input

- [ ] Look at POSIX Standards

## TODO POSIX Standards
- [ ] Use termios to configure the terminal.
- [x] Use fork, execvp, and waitpid for process control.
- [ ] Use select() or poll() for non-blocking input handling.
- [x] Use signal() to handle interrupts (SIGINT, SIGTSTP).
- [ ] Use getenv() and setenv() for managing environment variables.
- [ ] Avoid GNU-specific extensions (stick to man *3p* functions).

## LINKS
- [Build your own shell](https://github.com/tokenrove/build-your-own-shell)
