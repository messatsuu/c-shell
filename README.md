# C SHELL

Small shell project to learn more about C. Might actually make this halfway POSIX compliant.

## TODOs ordered by priority

### Input and Line Editing
- [ ] Raw character-by-character input handling
- [ ] Cursor movement (left/right arrows)
- [ ] Backspace/delete support
- [ ] Insert/overwrite modes (optional)
- [ ] Word movement (Ctrl+Left / Ctrl+Right)
- [ ] Clear line (Ctrl+L)
- [ ] Home/End key support

- [ ] Multi-line input support
- [ ] Handling input past screen width (scrolling text view)

#### History Management
- [x] Input history storage (in-memory)
- [ ] Traverse history (up/down arrows)
- [ ] Edit historical entries
- [ ] Prevent duplicate entries (optional)
- [ ] Persistent history file (optional)

### Keybindings
- [ ] Emacs-like bindings (Ctrl-A, Ctrl-E, Ctrl-K, Ctrl-U, Ctrl-W, etc.)
- [ ] Ctrl-D to exit shell
- [ ] Ctrl-C to cancel input line
- [ ] Optional Vi-mode keybindings

### Auto-completion
- [ ] Trigger on TAB
- [ ] Complete commands
- [ ] Complete file paths
- [ ] Custom completers (e.g. environment vars, functions)

### Terminal & Display
- [ ] Disable canonical mode & echo using `termios`
- [ ] Interpret escape sequences (arrow keys, Home/End, etc.)
- [ ] Track and move cursor within terminal
- [ ] Redraw prompt and input efficiently
- [ ] Handle terminal resizing (`SIGWINCH`)

###  Signal Handling
- [ ] Properly handle `SIGINT` (Ctrl-C)
- [ ] Handle `SIGTSTP` (Ctrl-Z) and backgrounding
- [ ] Restore terminal state on crash or exit

### Multibyte / UTF-8 Support
- [ ] Handle Unicode input and display width
- [ ] Support for combining characters or wide characters (e.g. Chinese)

- [ ] Look at POSIX Standards

## LINKS
- [Build your own shell](https://github.com/tokenrove/build-your-own-shell)
