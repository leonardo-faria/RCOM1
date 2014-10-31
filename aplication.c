#include "aplication.h"
int create_data_package(int n, unsigned char* data, int data_size,
		unsigned char** package) {

	(*package) = malloc(data_size + 4);
	(*package)[0] = 0x01;
	(*package)[1] = n;
	(*package)[2] = data_size / DATA_SIZE;
	(*package)[3] = data_size % DATA_SIZE;
	int i;
	for (i = 0; i < data_size; ++i)
		(*package)[i + 4] = data[i];

	return data_size + 4;
}

int create_control_package(unsigned char type, unsigned char* name,
		unsigned int name_size, unsigned int file_size, unsigned char** package) {
	unsigned char size_str[100];
	int size_size = 0;

	while (file_size > 0) {
		size_str[size_size++] = file_size % 256;
		file_size = file_size / 256;
	}
	(*package) = malloc(4 + size_size + name_size + 1);
	(*package)[0] = type;
	(*package)[1] = 1;
	(*package)[2] = name_size;

	int i;
	for (i = 0; i < name_size; i++)
		(*package)[3 + i] = name[i];

	(*package)[3 + i++] = 0;
	(*package)[3 + i] = size_size;
	for (i = 0; i < size_size; i++)
		(*package)[5 + name_size + i] = size_str[size_size - i - 1];
	return 5 + size_size + name_size;
}

//outra cena. as respostas penso que só deviam ser enviadas depois de confirmar se o que estamos a ler estar bem. se estiver errado já enviamos a resposta :D que eu tenha percebido não é suposto.
int alread(int fd) {
	unsigned char* package;
	package = malloc(DATA_SIZE);
	int pack_size;

	char fileName1, fileName2;
	int fileSize1, fileSize2;

	//read start control packet ->implementar os timeouts!
	while (1) {
		pack_size = llread(fd, &package);

		if (controlPackageStateMachine(package, 2, &fileName1, &fileSize1)
				== 0) {
			printf("Start package received!\n");
			break;

			//enviar a resposta com rr -> o llread já o faz
		} else {
			printf("Error while reading start package... Exiting!\n");
			exit(-1);
		}
	}

	//read the data packets -> como sabemos durante quanto tempo fazemos isto?
	int i = 0;
	int numPackets; //eu sei que já vi isto em algum lado

	FILE *file = fopen(&fileName1, "w");

	while (i < numPackets) {
		pack_size = llread(fd, &package);
		unsigned char *data;
		int sequenceNumber; //o tal numero que não sei onde o guardaste

		if (dataPackageStateMachine(package, 1, sequenceNumber, data) == 0) {
			fwrite(data, sizeof(unsigned char),
					pack_size /*nao tenho a certeza deste mas como o llread retorna o tamanho de apenas dos dados ou do packet com o header? penso que o primeiro mas confirma pf*/,
					file);

			//resposta enviada RR.
		} else {
			printf("Error while reading data... Exiting!\n");
			exit(1);
		}
		i++;
	}

	//read the end control packet and compare with the start control packet ->implementar timeouts
	while (1) {
		pack_size = llread(fd, &package);

		if (controlPackageStateMachine(package, 3, &fileName2, &fileSize2)
				== 0) {

			if (fileSize1 == fileSize2 && fileName1 == fileName2) {
				printf("End package received!\n");
				break;
			} else {
				printf("The start and end package doesn't match... Exiting\n");
				exit(1);
			}

			//enviar a resposta com rr -> o llread já o faz
		} else {
			printf("Error while reading end package... Exiting!\n");
			exit(1);
		}
	}
}

//passamos o controlo para ver qual estamos a receber. as outras é onde guardo as variaveis nome e tamanho do ficheiro
int controlPackageStateMachine(unsigned char *frame, int control,
		char* fileName, int* fileSize) {
	int state = 0;
	unsigned i = 0;
	int L1, L2, T1, T2;

	unsigned char size[256];

	while (state != 7)
		switch (state) {
		case 0:
			if (frame[i++] == control)
				state++;
			else
				return -1;
			break;
		case 1:
			T1 = frame[i++];
			state++;
			break;
		case 2:
			L1 = frame[i++];
			state++;
			break;
		case 3: {
			int j;
			if (T1 == 0) { //tamanho do ficheiro
				for (j = 0; j < L1; j++)
					size[j] = frame[i++];
				(*fileSize) = atoi(size);
			} else if (T1 == 1) //nome do ficheiro
				for (j = 0; j < L1; j++)
					fileName[j] = frame[i++];
			else
				return -1;
		}
			break;
		case 4:
			T2 = frame[i++];
			state++;
			break;
		case 5:
			L2 = frame[i++];
			state++;
			break;
		case 6: {
			int j;
			if (T2 == 0) { //tamanho do ficheiro
				for (j = 0; j < L2; j++)
					size[j] = frame[i++];
				(*fileSize) = atoi(size);
			} else if (T2 == 1) //nome do ficheiro
				for (j = 0; j < L2; j++)
					fileName[j] = frame[i++];
			else
				return -1;
		}
			state++;
			break;
		}

	return 0;
}

//aqui o controlo normalmente o sequence number não sei como o guardaste. tenho aqui duvidas nisto.
int dataPackageStateMachine(unsigned char *frame, int control,
		int sequenceNumber, unsigned char *data) {
	unsigned state = 0;
	unsigned i = 0;
	int L1, L2;

	while (state != 5)
		switch (state) {
		case 0:
			if (frame[i++] == control)
				state++;
			else
				return -1;
			break;
		case 1:
			if (frame[i++] == sequenceNumber)
				state++;
			else
				return -1;
			break;
		case 2:
			L1 = frame[i++];
			state++;
			break;
		case 3:
			L2 = frame[i++];
			state++;
			break;
		case 4: {
			int j;
			data = malloc(256 * L1 + L2);
			for (j = 0; j < (256 * L1 + L2); j++)
				data[j] = frame[i++];
		}
			state++;
			break;
		}
}

void alwrite(int fd, char* file_name, int name_size) {
	DATA_SIZE = 256;
	int i, pack_size;
	unsigned char* pack;
	struct stat st;
	int file = open(file_name, O_RDONLY);
	stat(file_name, &st);
	printf("file size:%d\n", DATA_SIZE);

	int npacks = 2 + ((int) st.st_size) / ((int) DATA_SIZE);
	if (st.st_size % DATA_SIZE != 0)
		++npacks;

	printf("Npacks:%d\n", npacks);

	for (i = 0; i < npacks; i++) {
		printf("\n\nPACK %d:\n", i);
		if (i == 0)
			pack_size = create_control_package(2, (unsigned char*) file_name,
					name_size, st.st_size, &pack);
		else if (i == npacks - 1)
			pack_size = create_control_package(3, (unsigned char*) file_name,
					name_size, st.st_size, &pack);
		else {
			unsigned char* r = malloc(DATA_SIZE);
			int re = read(file, r, DATA_SIZE);
			printf("read %d bytes\n", re);
			pack_size = create_data_package((i - 1) % 256, r, re - 1, &pack);
		}
		llwrite(fd, pack, pack_size);

	}
}

int main(int argc, char** argv) {
	int fd, c, res;

	//TODO alterar este if!!!!!
	if ((argc < 3)
			|| ((strcmp("/dev/ttyS0", argv[1]) != 0)
					&& (strcmp("/dev/ttyS4", argv[1]) != 0)
					&& ((strcmp("0", argv[2]) == 0)
							|| (strcmp("1", argv[2]) == 1)))) {
		printf(
				"Usage:\tnserial SerialPort\n\tex: nserial /dev/ttySx y (x is the port number and y defines if transmiter (0) or receiver(1).\n");
		exit(1);
	}

	AppLayer apl;

	fd = open(argv[1], O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror(argv[1]);
		exit(-1);
	}
	timeout = 0;
	apl.fileDescriptor = fd;
	apl.mode = atoi(argv[2]);

	struct sigaction act;
	act.sa_handler = alarmhandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGALRM, &act, NULL);

	if (llopen(apl) < 0) {
		printf("Failed to write or read SET/UA! Exiting!\n");
		exit(-1);
	}

	if (llclose(apl) == 0) {
		tcsetattr(apl.fileDescriptor, TCSANOW, &oldtio);
		close(apl.fileDescriptor);
		exit(0);
	} else {
		printf("Failed to write or read DISC/UA! Exiting!\n");
		exit(-1);
	}
}
