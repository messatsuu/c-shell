# C SHELL

Small shell project to learn more about C. Interested in learning about POSIX standards.

## TODOs ordered by priority

### Input and Line Editing
- [x] Raw character-by-character input handling
- [x] Cursor movement (left/right arrows)
- [x] Backspace/delete support
- [ ] Insert/overwrite modes (optional)
- [x] Word movement (Ctrl+Left / Ctrl+Right)
- [x] Clear line (Ctrl+L)
- [ ] Home/End key support

- [ ] Multi-line input support
- [ ] Handling input past screen width (scrolling text view)

#### History Management
- [x] Input history storage (in-memory)
- [x] Traverse history (up/down arrows)
- [ ] Edit historical entries
- [x] Prevent duplicate entries (optional)
- [ ] Persistent history file (optional)

### Keybindings
- [x] Emacs-like bindings (Ctrl-A, Ctrl-E, Ctrl-K, Ctrl-U, Ctrl-W, etc.)
- [x] Ctrl-D to exit shell
- [x] Ctrl-C to cancel input line
- [ ] Optional Vi-mode keybindings

### Auto-completion
#### General
- [x] Trigger on TAB
- [ ] Complete commands
- [ ] Custom completers (e.g. environment vars, functions)
- [ ] Render elements in grid after certain threshold
#### File Auto-completion
- [x] Complete file paths
- [x] Display file-elements in different colors

### Terminal & Display
- [x] Disable canonical mode & echo using `termios`
- [x] Interpret escape sequences (arrow keys, Home/End, etc.)
- [x] Track and move cursor within terminal
- [x] Redraw prompt and input efficiently
- [ ] Handle terminal resizing (`SIGWINCH`)

###  Signal Handling
- [x] Properly handle `SIGINT` (Ctrl-C)
- [ ] Handle `SIGTSTP` (Ctrl-Z) and backgrounding
- [ ] Restore terminal state on crash or exit

### Multibyte / UTF-8 Support
- [ ] Handle Unicode input and display width
- [ ] Support for combining characters or wide characters (e.g. Chinese)

- [ ] Look at POSIX Standards

## LINKS
- [Build your own shell](https://github.com/tokenrove/build-your-own-shell)
