#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


#define BAUDRATE B38400
#define _POSIX_SOURCE 1
#define FALSE 0
#define TRUE 1

#define F 0x7E
#define A 0x03
#define C 0x07
#define AE 0x03
#define AR 0x01

#define TRANSMITER 0
#define RECEIVER 1

char UA[5],SET[5], DISCE[5], DISCR[5];

int falhas;
struct termios oldtio,newtio;

struct applicationLayer { 
	int fileDescriptor; /*Descritor correspondente à porta série*/ 
	int mode; /*TRANSMITTER | RECEIVER*/ 
}typedef AppLayer;

int llopen(AppLayer apl);

int llread(int fd, char * buffer) ;

int writeTramas(AppLayer apl);
int llwrite(int fd, char * buffer, int length);

int llclose(AppLayer apl);

int stateMachine (int fd, unsigned char trama[5]);

int read_t(int fd,unsigned char* uc, int n);

int write_t(int fd, unsigned char* uc, int n);