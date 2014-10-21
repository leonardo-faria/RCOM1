/*Non-Canonical Input Processing*/
#include "AppLayer.h"





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




int llwrite(int fd, char * buffer, int length);
/*
	buffer e length lido no main
*/
int llread(int fd, char * buffer);