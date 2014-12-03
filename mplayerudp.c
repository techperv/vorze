#include "mplayerudp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


int mplayerUdpOpen(int port) {
	int sock;
	struct sockaddr_in a;
	sock=socket(PF_INET,SOCK_DGRAM,IPPROTO_IP);
	a.sin_family=AF_INET;
	a.sin_addr.s_addr=inet_addr("127.0.0.1"); a.sin_port=htons(port);
	if (bind(sock ,(struct sockaddr *)&a, sizeof(a)) == -1) {
		printf("Can't bind listening socket to port %d\n", port);
		perror("bind");
		exit(1); 
	}
	return sock;
}

int cmpDouble(double f1, double f2) {
	double e=0.000001;
	if (f1+e>f2) return 1;
	if (f1-e<f2) return -1;
	return 0;
}

//#define DEBUG
//Timestamp is in 1/10th second.
int mplayerUdpGetTimestamp(int sock) {
	float ts;
	char buffer[1024];
	int n;
	n=read(sock, buffer, sizeof(buffer));
	if (strncmp(buffer, "bye", 3)==0) {
		return -1;
	}

#ifdef DEBUG
	buffer[n]=0;
	printf("Recv: %s\n", buffer);
#endif
	ts=atof(buffer);
	return (ts*10);
}

int mplayerUdpClose(int sock) {
	close(sock);
}
