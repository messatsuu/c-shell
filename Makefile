CC = clang
DEBUG_CFLAGS = -g -O0
EXECUTABLE_PATH = ./bin/main
SRC_FILES = main.c
OBJ_FILES = $(SRC_FILES:.c=.o)

# PHONY are commands that are not files
.PHONY: build run debug debug-test build-run

build:
	$(CC) $(SRC_FILES) -o $(EXECUTABLE_PATH)

run:
	$(EXECUTABLE_PATH)

build-run: build run

debug:
	$(CC) $(DEBUG_CFLAGS) $(SRC_FILES) -o $(EXECUTABLE_PATH)
	lldb $(EXECUTABLE_PATH)

debug-test:
	$(CC) $(DEBUG_CFLAGS) test.c -o $(EXECUTABLE_PATH)
	lldb $(EXECUTABLE_PATH)

r: run
b: build
br: build-run
