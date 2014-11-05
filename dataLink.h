#include <linux/types.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>

#define SET	0x03
#define UA	0x07	
#define DISC	0x0b

#define RR1	0x85
#define REJ1	0x81
#define RR0	0x05
#define REJ0	0x01

#define A	0x03
#define F	0x7e

#define ESCAPE 0x7D
#define XOR_ESCAPE 0x5D
#define XOR_FLAG 0x5E
#define STF_XOR_VALUE 0x20

#define BAUDRATE B38400
#define _POSIX_SOURCE 1

#define TRANSMITER 0
#define RECEIVER 1

struct termios oldtio, newtio;

int PACK_SIZE;
int DATA_SIZE;
int ns;

int bytesWritedReaded;

int timeout;
int falhas;
int textMode;
int numRetransmissoes;
int timeoutTime;

int timeout;
int falhas;

struct applicationLayer {
	int fileDescriptor; /*Descritor correspondente à porta série*/
	int mode; /*TRANSMITTER | RECEIVER*/
}typedef AppLayer;

int llwrite(int fd, unsigned char * buffer, int length);
int llread(int fd, unsigned char** buffer);
int llopen(AppLayer apl);
int llclose(AppLayer apl);

int stuffing(unsigned char* buf, int length, unsigned char** stufBuf);
int destuffing(unsigned char* buf, int length, unsigned char** unstBuf);

int create_control_frame(unsigned char control, unsigned char** frame);
int create_info_frame(int ns, unsigned char *packages, int packages_size,
		unsigned char** frame);

void alarmhandler(int signo);

int controlStateMachine(int fd, unsigned char trama[5]);
int infoStateMachine(unsigned char *frame, int length, unsigned char **package);
