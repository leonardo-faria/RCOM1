#include "dataLink.h"

int llwrite(int fd, unsigned char * buffer, int length) {
	unsigned char* stuffed;
	unsigned char* frame;

	int frame_size = create_info_frame(ns, buffer, length, &frame);

	stuffed = malloc(length * 2);
	int stuff_size = stuffing(frame, frame_size, &stuffed);
	falhas = 0;
	while (falhas < numRetransmissoes) {
		timeout = 0;
		int missing = stuff_size;
		int num = 0;
		while (missing > 0) {
			num = write(fd, stuffed, missing);
			if (textMode)
				printf("Wrote %d bytes\n", num);
			stuffed += num;
			missing -= num;
		}
		stuffed -= stuff_size;
		unsigned char* rr;
		if (ns == 0)
			create_control_frame(RR1, &rr);
		else
			create_control_frame(RR0, &rr);
		alarm(timeoutTime);
		if (controlStateMachine(fd, rr) == 0)
			break;
	}
	if (falhas >= numRetransmissoes)
		return -1;
	if (ns == 0)
		ns = 1;
	else
		ns = 0;
	return frame_size;
}

void send_reject(int fd) {
	if (textMode)
		printf("Writing REJ\n");
	unsigned char* rej;
	int missing;
	if (ns == 0)
		missing = create_control_frame(REJ1, &rej);
	else
		missing = create_control_frame(REJ0, &rej);

	int num = 0;

	while (missing > 0) {
		num = write(fd, rej, missing);
		rej += num;
		missing -= num;
	}
}

int llread(int fd, unsigned char** buffer) {
	int stuff_size;
	int pack_size;
	falhas = 0;
	while (1) {
		unsigned char *stuffed = malloc((PACK_SIZE + 4) * 2 + 2);
		alarm(timeoutTime);
		timeout = 0;
		stuff_size = 0;
		int start = 0;
		int stop = 0;

		while (timeout != 1 && !stop) {
			int r = read(fd, &stuffed[stuff_size], 1);
			if (r > 0) {
				if (stuff_size != 0 && stuffed[stuff_size] == F)
					stop = 1;
				else if (stuff_size == 0 && stuffed[0] == F)
					start = 1;
				if (start)
					stuff_size += r;
			}
		}
		if (timeout == 1) {
			timeout = 0;
			continue;
		}
		falhas = 0;
		alarm(0);
		unsigned char* frame;

		int frame_size = destuffing(stuffed, stuff_size, &frame);

		if (frame_size < 0) {
			//send_reject(fd);
			continue;
		}

		pack_size = infoStateMachine(frame, frame_size, buffer);

		if (pack_size < 0) {
			//send_reject(fd);
			continue;
		}

		if (pack_size >= 0) {
			unsigned char* rr;
			int missing;

			if (ns == 0)
				missing = create_control_frame(RR1, &rr);
			else
				missing = create_control_frame(RR0, &rr);

			int num = 0;

			while (missing > 0) {
				num = write(fd, rr, missing);
				rr += num;
				missing -= num;
			}
			break;
		}
	}
	if (ns == 0)
		ns = 1;
	else
		ns = 0;
	if (falhas < numRetransmissoes)
		return pack_size;
	return -1;
}

int stuffing(unsigned char* buf, int length, unsigned char** stufBuf) {
	unsigned i;
	unsigned j = 0;
	(*stufBuf) = malloc(length * 2 + 2);
	(*stufBuf)[j++] = F;

	for (i = 0; i < length; i++) {
		if (buf[i] == F || buf[i] == ESCAPE) {
			(*stufBuf)[j++] = ESCAPE;
			if (buf[i] == F)
				(*stufBuf)[j++] = XOR_FLAG;
			else
				(*stufBuf)[j++] = XOR_ESCAPE;
		} else
			(*stufBuf)[j++] = buf[i];
	}

	(*stufBuf)[j++] = F;
	return j;
}

int destuffing(unsigned char* buf, int length, unsigned char** unstBuf) {
	unsigned i = 0;
	unsigned j = 0;

	i = 1;
	(*unstBuf) = malloc(length - 2);
	while (i < length - 1) {
		if (buf[i] == ESCAPE) {
			if (buf[i + 1] == XOR_FLAG) {
				(*unstBuf)[j++] = F;
				i += 2;
			} else if (buf[i + 1] == XOR_ESCAPE) {
				(*unstBuf)[j++] = ESCAPE;
				i += 2;
			} else
				return -1;

		} else
			(*unstBuf)[j++] = buf[i++];
	}
	return j;
}

int create_control_frame(unsigned char control, unsigned char** frame) {
	(*frame) = malloc(5);
	(*frame)[0] = F;
	(*frame)[1] = A;
	(*frame)[2] = control;
	(*frame)[3] = (*frame)[1] ^ (*frame)[2];
	(*frame)[4] = F;
	return 5;
}

int create_info_frame(int ns, unsigned char *packages, int packages_size,
		unsigned char** frame) {
	(*frame) = malloc(packages_size + 4);
	(*frame)[0] = A;
	if (ns == 0)
		(*frame)[1] = 0;
	else
		(*frame)[1] = 0x40;
	(*frame)[2] = (*frame)[1] ^ (*frame)[0];
	int i;
	unsigned char bcc = 0;
	for (i = 0; i < packages_size; ++i) {
		(*frame)[i + 3] = packages[i];
		bcc ^= packages[i];
	}
	(*frame)[i + 3] = bcc;

	return packages_size + 4;
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

	falhas = 0;
	timeout = 0;

	if (apl.mode == 0) { //EMISSOR
		while (falhas < numRetransmissoes) {
			timeout = 0;
			unsigned char *set;
			int size = create_control_frame(SET, &set);

			int num = 0;
			int missing = size;
			if (textMode)
				printf("Sender is writing SET.\n");
			while (missing > 0) {
				num = write(apl.fileDescriptor, set, missing);
				set += num;
				missing -= num;
			}
			unsigned char *ua;
			create_control_frame(UA, &ua);
			alarm(timeoutTime);
			if (controlStateMachine(apl.fileDescriptor, ua) == 0) {
				if (textMode)
					printf("Read  UA.\n");
				break;
			}
		}
	} else { //RECETOR
		while (falhas < numRetransmissoes) {
			unsigned char *set;
			create_control_frame(SET, &set);

			timeout = 0;
			alarm(timeoutTime);

			if (textMode)
				printf("Reading SET\n");
			if (controlStateMachine(apl.fileDescriptor, set) == 0) {
				if (textMode)
					printf("SET received correctly. Receiver is writing UA.\n");

				unsigned char *ua;
				int size = create_control_frame(UA, &ua);
				int num = 0;
				int missing = size;

				while (missing > 0) {
					num = write(apl.fileDescriptor, ua, missing);
					ua += num;
					missing -= num;
				}
				break;
			}
		}
	}
	if (falhas >= numRetransmissoes)
		return -1; //erro
	else
		return 0; //sucesso
}

int llclose(AppLayer apl) {
	int num;
	falhas = 0;
	timeout = 0;
	if (apl.mode == 0) {
		while (falhas < numRetransmissoes) {
			timeout = 0;
			if (textMode)
				printf("Writing DISC\n");
			unsigned char *disc;
			int size = create_control_frame(DISC, &disc);
			num = 0;
			int missing = size;

			while (missing > 0) {
				num = write(apl.fileDescriptor, disc, missing);
				disc += num;
				missing -= num;
			}
			disc -= num;
			alarm(timeoutTime);
			if (controlStateMachine(apl.fileDescriptor, disc) == 0) {
				if (textMode)
					printf("DISC received. Sending UA!\n");
				unsigned char *ua;
				int size = create_control_frame(UA, &ua);
				num = 0;
				int missing = size;
				while (missing > 0) {
					num = write(apl.fileDescriptor, ua, missing);
					ua += num;
					missing -= num;
				}
				break;
			}
		}
	} else {
		unsigned char *disc;
		int size = create_control_frame(DISC, &disc);
		while (falhas < numRetransmissoes) {
			timeout = 0;
			alarm(timeoutTime);
			if (controlStateMachine(apl.fileDescriptor, disc) == 0) {
				if (textMode)
					printf("DISC received. Sending it again!\n");
				num = 0;
				int missing = size;

				while (missing > 0) {
					num = write(apl.fileDescriptor, disc, missing);
					disc += num;
					missing -= num;
				}

				unsigned char *ua;
				create_control_frame(UA, &ua);

				if (controlStateMachine(apl.fileDescriptor, ua) == 0) {
					if (textMode)
						printf("UA received!\n");
					break;
				}
			}
		}
	}

	if (num > 0)
		return 0; //sucesso
	else
		return -1; //erro
}
void alarmhandler(int signo) {
	if (textMode)
		printf("Failed to finish read\n");
	falhas++;
	timeout = 1;
}

int controlStateMachine(int fd, unsigned char trama[5]) {
	unsigned char temp[1];
	int state = 0;
	while (state != 5 && !timeout) {
		int r = read(fd, temp, 1);

		if (r > 0) {
			alarm(0);
			switch (state) {
			case 0:
				if (*temp == trama[0])
					state = 1;
				break;
			case 1:
				if (*temp == trama[1])
					state = 2;
				else
					return -1;
				break;
			case 2:
				if (*temp == trama[0])
					state = 1;
				else if (*temp == trama[2])
					state = 3;
				else
					return -1;
				break;
			case 3:
				if (*temp == trama[0])
					state = 1;
				else if (*temp == trama[3])
					state = 4;
				else
					return -1;
				break;
			case 4:
				if (*temp == trama[4])
					state = 5;
				else
					return -1;
				break;
			}
			alarm(timeoutTime);
		}
	}

	if (timeout)
		return -1;
	else
		return 0;
}

int infoStateMachine(unsigned char *frame, int length, unsigned char **package) {
	int state = 0;
	int i;
	while (state != 4) {
		switch (state) {
		case 0:
			if (frame[0] == A)
				state++;
			else
				return -1;
			break;
		case 1:
			if ((ns == 0 && frame[1] == 0x00) || (ns == 1 && frame[1] == 0x40))
				state++;
			else
				return -2;
			break;
		case 2:
			if (frame[0] ^ frame[1] == frame[2])
				state++;
			else
				return -3;
			break;
		case 3: {
			unsigned char bcc = frame[3];

			for (i = 4; i < length - 1; i++) {
				bcc ^= frame[i];
			}
			if (bcc == frame[length - 1])
				state++;
			else
				return -4;
		}
			break;

		}
	}
	(*package) = malloc(length - 4);
	for (i = 3; i < length; i++) {
		(*package)[i - 3] = frame[i];
	}
	return length - 4;
}
