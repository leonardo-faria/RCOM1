#include "AppLayer.h"


int llopen(AppLayer apl)
{
	int fd=apl.fileDescriptor;

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
	return 
}

int llclose(int fd)
{
	
}
