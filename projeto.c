/*Non-Canonical Input Processing*/
#include "AppLayer.h"





int main(int argc, char** argv)
{
	int fd;
	if ( (argc < 2) || 
		((strcmp("/dev/ttyS0", argv[1])!=0) && 
			(strcmp("/dev/ttyS1", argv[1])!=0) )) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}



	fd = open(argv[1], O_RDWR | O_NOCTTY );
	if (fd <0) 
	{
		perror(argv[1]);
		exit(-1);
	}

    llopen(fd,0);

    csetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}


