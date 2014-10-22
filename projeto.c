#include "AppLayer.h"

void alarmhandler(int signo) {
	printf("Failed to finish write/read\n");

	if(++falhas==3)
	{
		printf("Failed 3 times\nExiting program\n");
		exit(1);
	}
	alarm(3);
}


int main(int argc, char** argv)
{
	int fd;
	if ( (argc < 2) || 
		((strcmp("/dev/ttyS0", argv[1])!=0) && 
			(strcmp("/dev/ttyS4", argv[1])!=0) )) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0\n");
		exit(1);
	}

	fd = open(argv[1], O_RDWR | O_NOCTTY );
	if (fd <0) 
	{
		perror(argv[1]);
		exit(-1);
	}

	struct sigaction act;
	act.sa_handler = alarmhandler;
	sigemptyset (&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGALRM, &act, NULL);

	llopen(fd,0);
	//csetattr(fd,TCSANOW,&oldtio);
	//close(fd);
	llclose(fd);
	return 0;
}


