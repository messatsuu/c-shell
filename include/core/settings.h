#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct {
    bool track_history;                 // whether to track the history
    unsigned int history_count;         // Number of elements saved in the history
    bool debug_mode;                    // Show debug info about parsed tokens and AST-tree
    bool no_file_override_on_redirect;  // If we redirect output to an existing file, do not override and throw warning
    bool exit_on_error;                 // If any command fails, exit immediately
    bool auto_cd;                       // Automatically cd to directory if argv[0] is a path
} Settings;

void init_settings();
void cleanup_settings();

#endif
