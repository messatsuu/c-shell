# PHONY makes sure that make doesn't confuse target-names with potential filenames
.PHONY: all clean test build run build-test run-test build-debug build-test-debug b r br bd btd bt rt brt

# GENERAL
CC = clang
EXECUTABLE_PATH = ./bin/main
EXECUTABLE_DEBUG_PATH = ./bin/main-debug

# FLAGS
CFLAGS = -std=c23 -O3 -I$(INC_DIR) -D_POSIX_C_SOURCE=200809L \
		-fsanitize=address,undefined -Wpedantic -Wformat=2 -Wno-unused-parameter \
		-Wall -Werror -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
		-Wredundant-decls -Wnested-externs -Wmissing-include-dirs

CFLAGS_DEBUG = -std=c23 -Og -g -I $(INC_DIR) -D_POSIX_C_SOURCE=200809L
TEST_WRAPPER_FLAGS = -Wl,--wrap=run_execvp,--wrap=get_host_name
# TODO: $NIX_LDFLAGS provided by nix develop-shell, is this a good idea?
LDFLAGS ?= $(shell echo $$NIX_LDFLAGS) -lcshread

# DIRS
SRC_DIR = ./src
INC_DIR = ./include
TEST_INC_DIR = ./tests/include/

# FILES
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

SRC_TEST_FILES = $(filter-out $(SRC_DIR)/main.c, $(SRC_FILES)) $(shell find tests -name '*.c')


# Standard Targets
all: build
clean:
	$(shell rm ./bin/*)
test: build-test run-test run-bats-test

# Main Targets
build:
	$(CC) $(SRC_FILES) $(CFLAGS) $(LDFLAGS) -o $(EXECUTABLE_PATH)

run:
	$(EXECUTABLE_PATH)

# Unit Testing Targets
build-test:
	$(CC) $(SRC_TEST_FILES) $(CFLAGS_DEBUG) $(TEST_WRAPPER_FLAGS) -I$(INC_DIR) -I$(TEST_INC_DIR) -lcshread -lcmocka -o ./bin/test

run-test:
	./bin/test

run-bats-test:
	bats tests/bats-core/*

# Debug Targets
build-debug:
	$(CC) $(SRC_FILES) $(CFLAGS_DEBUG) $(LDFLAGS) -o $(EXECUTABLE_DEBUG_PATH)

build-test-debug:
	$(CC) $(SRC_TEST_FILES) $(CFLAGS_DEBUG) $(TEST_WRAPPER_FLAGS) -I$(INC_DIR) -I$(TEST_INC_DIR) -lcshread -lcmocka -o ./bin/test-debug

# Aliases
b: build
r: run
br: build run

bd: build-debug
btd: build-test-debug

bt: build-test
rt: run-test
brt: build-test run-test
