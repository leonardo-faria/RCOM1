#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include "dataLink.h"

int create_control_package(unsigned char type, unsigned char* name,
		unsigned int name_size, unsigned int file_size, unsigned char** package);

int create_data_package(int n, unsigned char* data, int data_size,
		unsigned char** package);

int alread(int fd);

void alwrite(int fd, char* file_name, int name_size);

int controlPackageStateMachine(unsigned char *frame, int control,
		char** fileName, int* fileSize);

int dataPackageStateMachine(unsigned char *frame, int control,
		int sequenceNumber, unsigned char **data);
