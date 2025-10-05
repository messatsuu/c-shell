# PHONY are commands that are not files
.PHONY: build run debug debug-test build-run

# GENERAL
CC = clang
EXECUTABLE_PATH = ./bin/main
EXECUTABLE_DEBUG_PATH = ./bin/main-debug

# FLAGS
CFLAGS = -std=c23 -Wall -Werror -I$(INC_DIR) -D_POSIX_C_SOURCE=200809L
CFLAGS_DEBUG = -std=c23 -g -O0 -I$(INC_DIR) -D_POSIX_C_SOURCE=200809L
TEST_WRAPPER_FLAGS = -Wl,--wrap=run_execvp,--wrap=get_host_name
# TODO: provided by nix develop-shell, is this a good idea?
LDFLAGS ?= $(shell echo $$NIX_LDFLAGS)

# DIRS
SRC_DIR = ./src
INC_DIR = ./include
TEST_INC_DIR = ./tests/include/

# FILES
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

SRC_TEST_FILES = $(filter-out $(SRC_DIR)/main.c, $(SRC_FILES)) $(shell find tests -name '*.c')

# Main Targets
# TODO: add -fsanitize to CFLAGS once mem-leaks are addressed
build:
	$(CC) $(SRC_FILES) -o $(EXECUTABLE_PATH) $(LDFLAGS) $(CFLAGS) -fsanitize=address,undefined -lcshread

run:
	$(EXECUTABLE_PATH)

# Unit Testing Targets
build-test:
	$(CC) $(SRC_TEST_FILES) -lcmocka -o ./bin/test -isystem $(INC_DIR) -isystem $(TEST_INC_DIR) $(CFLAGS) $(TEST_WRAPPER_FLAGS) -lcshread

run-test:
	./bin/test

# Debug Targets
build-debug:
	$(CC) $(abspath $(SRC_FILES)) -o $(EXECUTABLE_DEBUG_PATH) $(LDFLAGS) $(CFLAGS_DEBUG) -lcshread

build-test-debug:
	$(CC) $(SRC_TEST_FILES) -lcmocka -o ./bin/test-debug -isystem $(INC_DIR) -isystem $(TEST_INC_DIR) $(CFLAGS_DEBUG) $(TEST_WRAPPER_FLAGS) -lcshread

# Aliases
b: build
r: run
br: build run

bd: build-debug
btd: build-test-debug

bt: build-test
rt: run-test
brt: build-test run-test
