CC = clang
DEBUG_CFLAGS = -g -O0
EXECUTABLE_PATH = ./bin/main

SRC_DIR = ./src
INC_DIR = ./include

SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/shell.c $(SRC_DIR)/command.c $(SRC_DIR)/process.c
OBJ_FILES = $(SRC_FILES:.c=.o)

# PHONY are commands that are not files
.PHONY: build run debug debug-test build-run

build:
	$(CC) $(SRC_FILES) -o $(EXECUTABLE_PATH) -isystem $(INC_DIR)

run:
	$(EXECUTABLE_PATH)

debug:
	$(CC) $(DEBUG_CFLAGS) $(SRC_FILES) -o $(EXECUTABLE_PATH)
	gdb -tui $(EXECUTABLE_PATH)

build-test:
	$(CC) $(DEBUG_CFLAGS) test.c -o ./bin/test

run-test:
	./bin/test

debug-test:
	gdb $(EXECUTABLE_PATH)

r: run
b: build
b: build
br: build run
bd: build debug

bt: build-test run-test
bdt: debug-test
