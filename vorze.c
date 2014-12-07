/*
This connects to the Vorze over the USB stick that comes with it. It's *NOT* a generic
BTLE implementation, so it'll *only* work with the provided stick.
*/
#include "vorze.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <glob.h>

#define BAUDRATE B19200
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define VORZE_VID "10c4"
#define VORZE_PID "897c"

//This is a pretty evil way to figure out the serial port the stick thingamajig is connected to... 
//but hey, it works. If you're on Linux. A recent version.
int vorzeDetectPort(char *serport) {
	int ix;
	int found=0;
	char buff[1024];
	char id[1024];
	glob_t globbuf;
	glob_t globbuf2;
	FILE *f;

	//Iterate over each USB device.
	glob("/sys/bus/usb/devices/*/", 0, NULL, &globbuf);
	for (ix=0; ix<globbuf.gl_pathc; ix++) {
		//Check VID
		sprintf(buff, "%s/idVendor", globbuf.gl_pathv[ix]);
		f=fopen(buff, "r");
		if (f==NULL) continue;
		fgets(id, sizeof(id), f);
		fclose(f);
		if (strncmp(id, VORZE_VID, 4)!=0) continue;

		//check PID
		sprintf(buff, "%s/idProduct", globbuf.gl_pathv[ix]);
		f=fopen(buff, "r");
		if (f==NULL) continue;
		fgets(id, sizeof(id), f);
		fclose(f);
		if (strncmp(id, VORZE_PID, 4)!=0) continue;

		//Grab the name of the file that lives at
		//sys/bus/usb/devices/x-y\:1.0/ttyUSB*/
		sprintf(buff, "%s*:1.0/ttyUSB*", globbuf.gl_pathv[ix]);
		printf("glob %s\n", buff);
		glob(buff, 0, NULL, &globbuf2);
		if (globbuf2.gl_pathc!=0) {
			char *p=strstr(globbuf2.gl_pathv[0], "ttyUSB");
			if (p!=NULL) {
				sprintf(serport, "/dev/%s", p);
				printf("Found Vorze USB stick at %s, dev name %s\n", globbuf.gl_pathv[ix], p);
				found=1;
			}
		}
		globfree(&globbuf2);
	}
	globfree(&globbuf);
	if (!found) printf("No Forze USB stick found.\n");
	return found;
}


int vorzeOpen(char *port) {
	struct termios newtio;
	fd_set rfds;
	struct timeval tv;
	int com;
	int status;
	
	com=open(port, O_RDWR | O_NOCTTY | CRTSCTS );
	if (com <0) {
		perror("Opening comport");
		return(-1); 
	}
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = (CS8 | CREAD | CLOCAL);
	newtio.c_lflag = 0;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	cfsetospeed(&newtio,BAUDRATE);

	newtio.c_cc[VTIME]    = 0;   // inter-character timer unused 
	newtio.c_cc[VMIN]     = 1;   // blocking read until 1 chars received 

	tcflush(com,TCIFLUSH);
	tcsetattr(com,TCSANOW,&newtio);
	ioctl(com, TIOCMGET, &status);
	status |= TIOCM_RTS;
	status |= TIOCM_DTR;
	status |= TIOCM_CTS;
	ioctl(com, TIOCMSET, &status);
	vorzeSet(com, 0, 0);
	return com;
}

int vorzeSet(int handle, int v1, int v2) {
	char buff[3]={1,1,0};
	buff[2]=(v1?0x80:0)|v2;
	write(handle, buff, 3);
	printf("V1=%d V2=%d\n", v1, v2);
}

int vorzeClose(int handle) {
	vorzeSet(handle, 0, 0);
	close(handle);
	return;
}
