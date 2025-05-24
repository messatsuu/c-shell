# PHONY are commands that are not files
.PHONY: build run debug debug-test build-run

CC = clang
CFLAGS = -std=c11 -Wall -Werror -isystem $(INC_DIR) -D_POSIX_C_SOURCE=200809L
DEBUG_CFLAGS = -g -O0
EXECUTABLE_PATH = ./bin/main
EXECUTABLE_DEBUG_PATH = ./bin/main-debug

SRC_DIR = ./src
INC_DIR = ./include
TEST_INC_DIR = ./tests/include/

SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

INC_TEST_DIR = ./tests/include
SRC_TEST_FILES = $(filter-out $(SRC_DIR)/main.c, $(SRC_FILES))
TEST_FILES = $(wildcard tests/src/*.c)
TEST_WRAPPER_AGS = -Wl,--wrap=run_execvp,--wrap=gethostname

# Main Targets
build:
	$(CC) $(SRC_FILES) -o $(EXECUTABLE_PATH) $(CFLAGS)

run:
	$(EXECUTABLE_PATH)

# Unit Testing Targets
build-test:
	$(CC) $(TEST_FILES) $(SRC_TEST_FILES) -lcmocka -o ./bin/test -isystem $(INC_DIR) -isystem $(TEST_INC_DIR) $(TEST_WRAPPER_AGS)

run-test:
	./bin/test

# Debug Targets
build-debug:
	$(CC) $(DEBUG_CFLAGS) $(SRC_FILES) -o $(EXECUTABLE_DEBUG_PATH) -isystem $(INC_DIR)

build-test-debug:
	$(CC) $(DEBUG_CFLAGS) $(TEST_FILES) $(SRC_TEST_FILES) -lcmocka -o ./bin/test-debug -isystem $(INC_DIR) -isystem $(TEST_INC_DIR) $(TEST_WRAPPER_AGS)

# Aliases
b: build
r: run
br: build run

bd: build-debug
btd: build-test-debug

bt: build-test
rt: run-test
brt: build-test run-test
