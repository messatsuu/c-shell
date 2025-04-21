#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static struct termios orig_termios;

void move_cursor_left() {
    printf("\x1b[D");
    fflush(stdout);
}

void move_cursor_right() {
    printf("\x1b[C");
    fflush(stdout);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN); // Disable echo and canonical mode
    raw.c_iflag &= ~(IXON); // Disable XON/XOFF flow control
    raw.c_oflag &= ~OPOST;  // Disable output processing
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
