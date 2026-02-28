#include "../include/utility.h"

#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// We mock get_host_name() (function in prompt.c), since the host is context-dependant
ssize_t __wrap_get_host_name(char *name, size_t len) {
    strcpy(name, "test-host");
    return 0;
}
