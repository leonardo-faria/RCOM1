CC=gcc
CFLAGS=-Wall
SOURCES=AppLayer.c projeto.c
EXECUTABLE=rcom

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE).
