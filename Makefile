CC = clang
CFLAGS = -std=c11 -Wall -Werror -isystem $(INC_DIR)
DEBUG_CFLAGS = -g -O0
EXECUTABLE_PATH = ./bin/main

SRC_DIR = ./src
INC_DIR = ./include

SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/shell.c $(SRC_DIR)/command.c $(SRC_DIR)/process.c $(SRC_DIR)/history.c $(SRC_DIR)/utility.c
OBJ_FILES = $(SRC_FILES:.c=.o)

# PHONY are commands that are not files
.PHONY: build run debug debug-test build-run

build:
	$(CC) $(SRC_FILES) -o $(EXECUTABLE_PATH) $(CFLAGS)

run:
	$(EXECUTABLE_PATH)

debug:
	$(CC) $(DEBUG_CFLAGS) $(SRC_FILES) -o $(EXECUTABLE_PATH) -isystem $(INC_DIR)

build-test:
	$(CC) tests/test_ps1.c  $(SRC_DIR)/shell.c $(SRC_DIR)/command.c $(SRC_DIR)/process.c $(SRC_DIR)/history.c $(SRC_DIR)/utility.c -lcmocka -o ./bin/test

run-test:
	./bin/test

debug-test:
	gdb ./bin/test

# Aliases
r: run
b: build
b: build
br: build run
bd: debug

brt: build-test run-test
bdt: debug-test
