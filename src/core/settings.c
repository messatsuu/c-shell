#include "core/settings.h"
#include "utility.h"
#include <cshread/history.h>
#include <stdlib.h>

Settings *settings = nullptr;

#define DEFAULT_HISTORY_SIZE 100;
#ifdef DEBUG
    #define DEBUG_MODE 1
#else
    #define DEBUG_MODE 0
#endif

void init_settings() {
    settings = callocate(1, sizeof(Settings), true);
    settings->track_history = true;
    settings->history_count = DEFAULT_HISTORY_SIZE;
    settings->debug_mode = DEBUG_MODE;
    settings->no_file_override_on_redirect = false;
    settings->exit_on_error = false;
    settings->auto_cd = true;
}

void cleanup_settings() {
    free(settings);
}

void apply_settings() {
    if (settings == nullptr) {
        init_settings();
    }

    cshr_set_history_limit(settings->history_count);
}
