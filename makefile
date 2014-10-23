CC=gcc
CFLAGS=
SOURCES= projeto.c
EXECUTABLE=rcom

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)
