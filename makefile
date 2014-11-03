CC=gcc
CFLAGS=
SOURCES= aplication.c dataLink.c
EXECUTABLE=rcom

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE) -lm
