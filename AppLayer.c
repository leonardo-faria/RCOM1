#include "AppLayer.h"

#define AE 0x03
#define AR 0x01

char UA[5],SET[5];
int mode;


int llopen(int fd,int mMode)
{
	
	UA[0] = F;
	UA[1] = A;
	UA[2] = C;
	UA[3] = UA[1]^UA[2];
	UA[4] = F;

	SET[0] = F;
	SET[1] = A;
	SET[2] = 0x03;
	SET[3] = SET[1]^SET[2];
	SET[4] = F;

	int c, res;
	struct termios oldtio,newtio;
	char buf[255];

	mode=mMode;
	


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

    if(apl.status==TRANSMITTER)
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

    	state_machine(fd, UA);
    }
    else
    {
    	state_machine(fd,SET);

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
    return fd;
}

int state_machine(int fd, unsigned char trama[5])
{
	unsigned char *temp;
	int state=0;
	while(state!=5)
	{
		switch(state)
		{
			case 0:
			read(fd,temp, 1);
			if(*temp==trama[0])
				state=1;
			break;
			case 1:
			read(fd,temp, 1);
			if(*temp==trama[1])
				state=2;
			else if(*temp!=trama[0])
				state=0;
			break;	
			case 2:
			read(fd,temp, 1);
			if(*temp==trama[0])
				state=1;
			else if(*temp==trama[2])
				state=3;
			else state=0;
			break;	
			case 3:
			read(fd,temp, 1);
			if(*temp==trama[0])
				state=1;
			else if(*temp==trama[3])
				state=4;
			else state=0;
			break;	
			case 4:

			read(fd,temp, 1);
			if(*temp==trama[4])
				state=5;
			else state=0;
			break;	
		}
	}
	//TODO timeouts!!
	return 0;
}

int llclose(int fd)
{
	if(mode==TRANSMITTER)
	{
		unsigned char DISC[5];
		DISC[0] = F;
		DISC[1] = AE;
		DISC[2] = 0x0B;
		DISC[3] = DISC[1]^DISC[2];
		DISC[4] = F;

		unsigned char UA[5];
		UA[0] = F;
		UA[1] = A;
		UA[2] = C;
		UA[3] = UA[1]^UA[2];
		UA[4] = F;

		unsigned char *aux = DISC;

		int size = sizeof(DISC) / sizeof(unsigned char);
		int num = 0;
		int missing = size;

		while(missing > 0) {
			num = write(fd,aux,missing);
			aux += num;
			missing -= num;
		}

		if(state_machine(fd, DISC) == 0) {
			aux = UA;

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
	else
	{
		unsigned char DISC[5];
		DISC[0] = F;
		DISC[1] = AR;
		DISC[2] = 0x0B;
		DISC[3] = DISC[1]^DISC[2];
		DISC[4] = F;

		state_machine(fd, DISC);

		unsigned char *aux = DISC;

		int size = sizeof(UA) / sizeof(unsigned char);
		int num = 0;
		int missing = size;

		while(missing > 0) {
			num = write(fd,aux,missing);
			aux += num;
			missing -= num;
		}
	}

	if(close(fd) == 0)
		return 1;
	else
		return -1;

	return -1;
}
