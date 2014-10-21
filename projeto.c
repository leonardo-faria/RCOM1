/*Non-Canonical Input Processing*/

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


int llopen(AppLayer apl);

struct applicationLayer { 
	int fileDescriptor; /*Descritor correspondente à porta série*/ 
	int status; /*TRANSMITTER | RECEIVER*/ 
}typedef AppLayer; 

int main(int argc, char** argv)
{
	int fd,c, res;
	struct termios oldtio,newtio;
	char buf[255];


	if ( (argc < 2) || 
		((strcmp("/dev/ttyS0", argv[1])!=0) && 
			(strcmp("/dev/ttyS1", argv[1])!=0) )) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
	exit(1);
}



fd = open(argv[1], O_RDWR | O_NOCTTY );
if (fd <0) {perror(argv[1]); exit(-1); }


if ( tcgetattr(fd,&oldtio) == -1) { 
	perror("tcgetattr");
	exit(-1);
}

AppLayer apl;
apl.fileDescriptor = fd;
    apl.status = RECEIVER; //TODO mudar isto

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0;   
    newtio.c_cc[VMIN] = 1;



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    	perror("tcsetattr");
    	exit(-1);
    }


    llopen(apl);

    csetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}


int llopen(AppLayer apl)
{
	int fd=apl.fileDescriptor;
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

	if(apl.status!=TRANSMITTER)
	{
		unsigned char *aux = SET;
		
		int size = sizeof(SET) / sizeof(unsigned char);
		int num = 0;
		int missing = size;

		while(missing > 0) {
			num = write(fd,aux,missing);
			aux += num;
			missing -= num;
		}

		unsigned char *temp;
		int state=0;
		while(state!=5)
		{
			switch(state)
			{
				case 0:
				read(fd,temp, 1);
				if(*temp==UA[0])
					state=1;
				break;
				case 1:
				read(fd,temp, 1);
				if(*temp==UA[1])
					state=2;
				else if(*temp!=UA[0])
					state=0;
				break;	
				case 2:
				read(fd,temp, 1);
				if(*temp==UA[0])
					state=1;
				else if(*temp==UA[2])
					state=3;
				else state=0;
				break;	
				case 3:
				read(fd,temp, 1);
				if(*temp==UA[0])
					state=1;
				else if(*temp==UA[3])
					state=4;
				else state=0;
				break;	
				case 4:
				
				read(fd,temp, 1);
				if(*temp==UA[4])
					state=5;
				else state=0;
				break;	
			}
		}
	}
	else
	{
		unsigned char *temp;
		int state=0;
		while(state!=5)
		{
			switch(state)
			{
				case 0:
				read(fd,temp, 1);
				if(*temp==SET[0])
					state=1;
				break;
				case 1:
				read(fd,temp, 1);
				if(*temp==SET[1])
					state=2;
				else if(*temp!=SET[0])
					state=0;
				break;	
				case 2:
				read(fd,temp, 1);
				if(*temp==SET[0])
					state=1;
				else if(*temp==SET[2])
					state=3;
				else state=0;
				break;	
				case 3:
				read(fd,temp, 1);
				if(*temp==SET[0])
					state=1;
				else if(*temp==SET[3])
					state=4;
				else state=0;
				break;	
				case 4:
				
				read(fd,temp, 1);
				if(*temp==SET[4])
					state=5;
				else state=0;
				break;	
			}
		}


		unsigned char *aux = UA;
		
		int size = sizeof(UA) / sizeof(unsigned char);
		int num = 0;
		int missing = size;

		while(missing > 0) {
			num = write(fd,aux,missing);
			aux += num;
			missing -= num;
		}
	}
}

int llwrite(int fd, char * buffer, int length);
/*
	buffer e length lido no main
*/
int llread(int fd, char * buffer);