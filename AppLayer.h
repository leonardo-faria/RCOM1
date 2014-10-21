
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>


#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define F 0x7E
#define A 0x03
#define C 0x07

#define TRANSMITTER 0
#define RECEIVER 1

volatile int STOP=FALSE;

struct applicationLayer { 
	int fileDescriptor; /*Descritor correspondente à porta série*/ 
	int status; /*TRANSMITTER | RECEIVER*/ 
}typedef AppLayer; 


	char UA[5],SET[5];
	UA[0] = F;
	UA[1] = A;
	UA[2] = C;
	UA[3] = UA[1]^UA[2];
	UA[4] = F;

	SET[0] = F;
	SET[1] = A;
	SET[2] = 0x03;
	SET[3] = UA[1]^SET[2];
	SET[4] = F;


int llopen(AppLayer apl);
int llwrite(int fd, char * buffer, int length);
int llread(int fd, char * buffer);
int llclose(int fd);
