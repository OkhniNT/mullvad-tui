CC=clang
CFLAGS=-Wall
CLIBS=-lncurses

all: build run clean

build:
	$(CC) $(CFLAGS) $(CLIBS) main.c -o debug.o

run:
	./debug.o

clean:
	rm debug.o

release:
	$(CC) $(CLIBS) main.c -o mulltui.o
