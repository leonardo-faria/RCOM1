
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define F 0x7E
#define A 0x03
#define C 0x07
#define AE 0x03
#define AR 0x01

#define TRANSMITTER 0
#define RECEIVER 1

char UA[5],SET[5];
int mode,falhas;

//volatile int STOP=FALSE;

struct applicationLayer { 
	int fileDescriptor; /*Descritor correspondente à porta série*/ 
	int status; /*TRANSMITTER | RECEIVER*/ 
}typedef AppLayer; 

int read_t(int fd,unsigned char* uc, int n);
int write_t(int fd, unsigned char* uc, int n);
void alarmhandler(int signo);
int llopen(int fd,int mMode);
int llwrite(int fd, char * buffer, int length);
int llread(int fd, char * buffer);
int llclose(int fd);
int state_machine(int fd, unsigned char trama[5]);
