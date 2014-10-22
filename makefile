CC=gcc
CFLAGS=
SOURCES=AppLayer.c projeto.c
EXECUTABLE=rcom

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)
