#include "projeto.h"

void initializeArrays () {
	UA[0] = F;
	UA[1] = A;
	UA[2] = C;
	UA[3] = UA[1] ^ UA[2];
	UA[4] = F;

	SET[0] = F;
	SET[1] = A;
	SET[2] = 0x03;
	SET[3] = SET[1] ^ SET[2];
	SET[4] = F;

	//TODO verificar estes valores
	DISCW[0] = F;
	DISCW[1] = AE;
	DISCW[2] = 0x0B;
	DISCW[3] = DISCW[1]^DISCW[2];
	DISCW[4] = F;

	DISCR[0] = F;
	DISCR[1] = AR;
	DISCR[2] = 0x0B;
	DISCR[3] = DISCR[1]^DISCR[2];
	DISCR[4] = F;
}

void alarmhandler(int signo) {
	printf("Failed to finish read\n");

	if (++falhas != 3)
		alarm(1);
}

int main(int argc, char** argv) {
	int fd, c, res;

	if ((argc < 3)
		|| ((strcmp("/dev/ttyS0", argv[1]) != 0)
			&& (strcmp("/dev/ttyS4", argv[1]) != 0)
			&& ((strcmp("0", argv[2]) == 0)
				|| (strcmp("1", argv[2]) == 1)))) {
		printf(
			"Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1 0 for transmiter, 1 for receiver\n");
		exit(1);
	}

	AppLayer apl;

	fd = open(argv[1], O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror(argv[1]);
		exit(-1);
	}
	apl.fileDescriptor = fd;
	apl.mode = atoi(argv[2]);

	falhas = 0;
	initializeArrays();

	struct sigaction act;
	act.sa_handler = alarmhandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGALRM, &act, NULL);

	llopen(apl);

	if(mode ==0)
		llwrite(apl);
	else
		llread(apl);

	//llclose(apl);
}

int llopen(AppLayer apl) {

	if (tcgetattr(apl.fileDescriptor, &oldtio) == -1) {
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 0; /* blocking read until 5 chars received */

	tcflush(apl.fileDescriptor, TCIOFLUSH);

	if (tcsetattr(apl.fileDescriptor, TCSANOW, &newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	if (apl.mode == 0) {
		while(1)
		{
			unsigned char *aux = SET;
			
			int size = sizeof(SET) / sizeof(unsigned char);
			int num = 0;
			int missing = size;
			printf("Writing SET\n");
			while (missing > 0) {
				num = write(apl.fileDescriptor, aux, missing);
				aux += num;
				missing -= num;
			}
			if(stateMachine(apl.fileDescriptor, UA)==0)
				break;
		}
	} else {
		stateMachine(apl.fileDescriptor, SET);
		
		unsigned char *aux = UA;

		int size = sizeof(UA) / sizeof(unsigned char);
		int num = 0;
		int missing = size;

		while (missing > 0) {
			num = write(apl.fileDescriptor, aux, missing);
			aux += num;
			missing -= num;

		}
	}
	return 0;
}

int llread(AppLayer apl) {

}

int llwrite(AppLayer apl) {

}

//TODO cada array tem de ter uma versao transmitter e receiver
int llclose(AppLayer apl) 
{
	if(mode==0)
	{	
		while(1)
		{
			printf("Writing DISC\n");
			unsigned char *aux = DISCW;

			int size = sizeof(DISCW) / sizeof(unsigned char);
			int num = 0;
			int missing = size;

			while(missing > 0)
			{
				num = write(apl.fileDescriptor,aux,missing);
				aux += num;
				missing -= num;
			}

			if(stateMachine(apl.fileDescriptor, DISCR) == 0)
			{
				aux = UA;

				int size = sizeof(UA) / sizeof(unsigned char);
				int num = 0;
				int missing = size;

				while(missing > 0) {
					num = write(apl.fileDescriptor,aux,missing);
					aux += num;
					missing -= num;
				}
				break;
			}
		}
	}
	else
	{
		stateMachine(apl.fileDescriptor, DISCW);

		unsigned char *aux = DISCR;

		int size = sizeof(UA) / sizeof(unsigned char);
		int num = 0;
		int missing = size;

		while(missing > 0) {
			num = write(apl.fileDescriptor,aux,missing);
			aux += num;
			missing -= num;
		}
	}

	tcsetattr(apl.fileDescriptor, TCSANOW, &oldtio);
	close(apl.fileDescriptor);
	return 0;
}

int stateMachine(int fd, unsigned char trama[5]) {
	unsigned char temp[1];
	int state = 0;
	while (state != 5) {
		if(falhas==3)
		{	
			falhas=0;
			return -1;
		}
		switch (state) {
			case 0:
			read_t(fd, temp, 1);
			if (*temp == trama[0])
				state = 1;
			break;
			case 1:
			read_t(fd, temp, 1);
			if (*temp == trama[1])
				state = 2;
			else if (*temp != trama[0])
				state = 0;
			break;
			case 2:
			read_t(fd, temp, 1);
			if (*temp == trama[0])
				state = 1;
			else if (*temp == trama[2])
				state = 3;
			else
				state = 0;
			break;
			case 3:
			read_t(fd, temp, 1);
			if (*temp == trama[0])
				state = 1;
			else if (*temp == trama[3])
				state = 4;
			else
				state = 0;
			break;
			case 4:

			read_t(fd, temp, 1);
			if (*temp == trama[4])
				state = 5;
			else
				state = 0;
			break;

		}
	}
	printf("chegou ao fim da stateMachine\n");
	return 0;
}

int read_t(int fd, unsigned char* uc, int n) {
	int r = -1;
	alarm(1);
	while (r <= 0 && falhas<3) {
		r = read(fd, uc, n);
	}
	alarm(0);
	if(r>0)
		falhas = 0;

	return r;
}

