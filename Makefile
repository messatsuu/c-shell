EXECUTABLE_PATH=./bin/main

build:
	clang main.c -o $(EXECUTABLE_PATH)

run:
	$(EXECUTABLE_PATH)

build-run: build run

r: build-run
