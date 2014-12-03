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

#define BAUDRATE B19200
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

//01 01 00 - reset
//01 01 82 - 1,2
//

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
