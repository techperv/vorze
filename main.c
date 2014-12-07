#include "mplayerudp.h"
#include "csv.h"
#include "vorze.h"
#include "js.h"
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

#define ACT_NONE 0
#define ACT_PLAY 1
#define ACT_RECORD 2
#define ACT_TEST 3

int main(int argc, char **argv) {
	CsvEntry *csv, *ent;
	char serport[1024];
	char csvfile[1024];
	char jsdev[1024];
	int js;
	int sock;
	int port=23867;
	int vorze;
	int offset=0;
	int ts=0;
	int action=ACT_NONE;
	int ix;

	strcpy(csvfile, "");
	strcpy(jsdev, "/dev/input/js0");
	strcpy(serport, "");


	for (ix=0; ix<argc; ix++) {
		if (!strcmp(argv[ix], "-c") && ix>argc+1) {
			ix++;
			strcpy(serport, argv[ix]);
		} else if (!strcmp(argv[ix], "-u") && ix>argc+1) {
			ix++;
			port=atoi(argv[ix]);
		} else if (!strcmp(argv[ix], "-o") && ix>argc+1) {
			ix++;
			offset=atoi(argv[ix]);
		} else if (!strcmp(argv[ix], "play") && ix>argc+1) {
			action=ACT_PLAY;
			ix++;
			strcpy(csvfile, argv[ix]);
		} else if (!strcmp(argv[ix], "record") && ix>argc+1) {
			action=ACT_RECORD;
			ix++;
			strcpy(csvfile, argv[ix]);
		} else if (!strcmp(argv[ix], "test")) {
			action=ACT_TEST;
		} else {
			action=ACT_NONE;
			printf("Error: Unknown arg %s\n", argv[ix]);
			break;
		}
	}
	
	if (action==ACT_NONE) {
		printf("Usage:\n");
		printf(" %s play file.csv [options]\n", argv[0]);
		printf(" %s record file.csv [options]\n", argv[0]);
		printf(" %s test [options]\n", argv[0]);
		printf(" -c serport: specify Vorze serial port (def: autodetect)\n");
		printf(" -u udpport: specify mplayer UDP port (def: 23867)\n");
		printf(" -o 1234: specify movie offset in decisec (def: 0)\n");
		exit(0);
	}
	
	if (strlen(serport)==0) vorzeDetectPort(serport);

	vorze=vorzeOpen(serport);
	if (vorze<=0) exit(1);

	if (action==ACT_PLAY) {
		csv=csvLoad(csvfile);
		if (csv==NULL) exit(1);
		sock=mplayerUdpOpen(port);
		if (sock<=0) exit(1);

		handleTs(0, csv, vorze);
		while(ts>=0) {
			ts=mplayerUdpGetTimestamp(sock)-offset;
			handleTs(ts, csv, vorze);
//			printf("Frame: %d\n", ts);
		}
		mplayerUdpClose(sock);
		csvFree(csv);
	}

	if (action==ACT_TEST) {
		int v1, v2;
		js=jsOpen(jsdev);
		while(1) {
			jsRead(js, &v1, &v2);
			vorzeSet(vorze, v1, v2);
		}
		jsClose(js);
	}

}