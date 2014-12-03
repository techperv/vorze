#include "mplayerudp.h"
#include "csv.h"
#include "vorze.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleTs(int ts, CsvEntry *csv, int vorzeHandle) {
	static char currV1=-1, currV2=-1;
	CsvEntry *ent;
	ent=csvGetForTs(csv, ts);
	if (currV1!=ent->v1 && currV2!=ent->v2) {
		vorzeSet(vorzeHandle, ent->v1, ent->v2);
	}
	currV1=ent->v1; currV2=ent->v2;
}

int main(int argc, char **argv) {
	CsvEntry *csv, *ent;
	char serport[512];
	int sock;
	int port=23867;
	int vorze;
	int ts=0;
	if (argc<2) {
		printf("Usage: %s file.csv [serial_port [udp_port]]\n");
		exit(0);
	}
	strcpy(serport, "/dev/ttyUSB0");
	if (argc>=3) strcpy(serport, argv[2]);
	if (argc>=4) port=atoi(argv[3]);
	csv=csvLoad(argv[1]);
	if (csv==NULL) exit(1);
	sock=mplayerUdpOpen(23867);
	if (sock<=0) exit(1);
	vorze=vorzeOpen(serport);
	if (vorze<=0) exit(1);

	handleTs(0, csv, vorze);
	while(ts>=0) {
		ts=mplayerUdpGetTimestamp(sock);
		handleTs(ts, csv, vorze);
		printf("Frame: %d\n", ts);
	}
	mplayerUdpClose(sock);
	csvFree(csv);
}