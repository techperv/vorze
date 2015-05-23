#include "mplayerudp.h"
#include "csv.h"
#include "vorze.h"
#include "js.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

void handleTs(int ts, CsvEntry *csv, int vorzeHandle) {
	static char currV1=-1, currV2=-1;
	CsvEntry *ent;
	ent=csvGetForTs(csv, ts);
	if (currV1!=ent->v1 || currV2!=ent->v2) {
		vorzeSet(vorzeHandle, ent->v1, ent->v2);
	}
	currV1=ent->v1; currV2=ent->v2;
}

#define ACT_NONE 0
#define ACT_PLAY 1
#define ACT_RECORD 2
#define ACT_TEST 3
#define ACT_PLAYSA 4
#define ACT_RECORDSA 5

volatile sig_atomic_t gotalarm=0;

void handle_alarm(int signum) {
	gotalarm=1;
	signal(SIGALRM, handle_alarm);
}

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


	for (ix=1; ix<argc; ix++) {
		if (!strcmp(argv[ix], "-c") && ix>argc+1) {
			ix++;
			strcpy(serport, argv[ix]);
		} else if (!strcmp(argv[ix], "-u") && ix<argc-1) {
			ix++;
			port=atoi(argv[ix]);
		} else if (!strcmp(argv[ix], "-o") && ix<argc-1) {
			ix++;
			offset=atoi(argv[ix]);
		} else if (!strcmp(argv[ix], "play") && ix<argc-1) {
			action=ACT_PLAY;
			ix++;
			strcpy(csvfile, argv[ix]);
		} else if (!strcmp(argv[ix], "record") && ix<argc-1) {
			action=ACT_RECORD;
			ix++;
			strcpy(csvfile, argv[ix]);
		} else if (!strcmp(argv[ix], "test")) {
			action=ACT_TEST;
		} else if (!strcmp(argv[ix], "playsa") && ix<argc-1) {
			action=ACT_PLAYSA;
			ix++;
			strcpy(csvfile, argv[ix]);
		} else if (!strcmp(argv[ix], "recordsa") && ix<argc-1) {
			action=ACT_RECORDSA;
			ix++;
			strcpy(csvfile, argv[ix]);
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
		printf(" %s playsa file.csv [options]\n", argv[0]);
		printf(" %s recordsa file.csv [options]\n", argv[0]);
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
			ts=mplayerUdpGetTimestamp(sock)+offset;
			printf("\rFrame: %5d  ", ts);
                        fflush(stdout);
			handleTs(ts, csv, vorze);
		}
		mplayerUdpClose(sock);
		csvFree(csv);
	}
	
	if (action==ACT_RECORD) {
		int v1, v2;
		int oldv1=-1, oldv2=-1;
		FILE *f;
		js=jsOpen(jsdev);
		sock=mplayerUdpOpen(port);
		if (sock<=0) exit(1);
		f=fopen(csvfile, "w");
		if (f==NULL) {
			perror(csvfile);
			exit(1);
		}
		while(ts>=0) {
			ts=mplayerUdpGetTimestamp(sock)+offset;
			jsRead(js, &v1, &v2);
			if (v1!=oldv1 || v2!=oldv2) {
				vorzeSet(vorze, v1, v2);
				oldv1=v1; oldv2=v2;
				fprintf(f, "%d,%d,%d\n", ts, v1, v2);
			} else {
				vorzeDoResendIfNeeded(vorze);
			}
		}
		fclose(f);
		jsClose(js);
	}
	
	if (action==ACT_TEST) {
		int v1, v2;
		int oldv1=-1, oldv2=-1;
		js=jsOpen(jsdev);
		while(1) {
			jsRead(js, &v1, &v2);
			if (v1!=oldv1 || v2!=oldv2) {
				vorzeSet(vorze, v1, v2);
				oldv1=v1; oldv2=v2;
			} else {
				vorzeDoResendIfNeeded(vorze);
			}
			usleep(100000);
		}
		jsClose(js);
	}

	if (action==ACT_PLAYSA) {
		csv=csvLoad(csvfile);
		if (csv==NULL) exit(1);
		sigset_t mask, oldmask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGALRM);
		sigprocmask(SIG_BLOCK, &mask, &oldmask);
		signal(SIGALRM, handle_alarm);
		const struct itimerval TENTHS = {{0, 100000}, {0, 100000}};
		if (setitimer(ITIMER_REAL, &TENTHS, NULL)) {
			exit(1);
		}

		handleTs(0, csv, vorze);
		while(ts>=0) {
			while(!gotalarm) {
				sigsuspend(&oldmask);
			}
			gotalarm = 0;
			sigprocmask(SIG_UNBLOCK, &mask, NULL);
			printf("\rFrame: %5d  ", ts);
                        fflush(stdout);
			handleTs(ts, csv, vorze);
                        ts++;
		}
		csvFree(csv);
	}

	if (action==ACT_RECORDSA) {
		int v1, v2;
		int oldv1=-1, oldv2=-1;
		FILE *f;
		js=jsOpen(jsdev);
		f=fopen(csvfile, "w");
		if (f==NULL) {
			perror(csvfile);
			exit(1);
		}
		sigset_t mask, oldmask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGALRM);
		sigprocmask(SIG_BLOCK, &mask, &oldmask);
		signal(SIGALRM, handle_alarm);
		const struct itimerval TENTHS = {{0, 100000}, {0, 100000}};
		if (setitimer(ITIMER_REAL, &TENTHS, NULL)) {
			exit(1);
		}

		while(ts>=0) {
			while(!gotalarm) {
				sigsuspend(&oldmask);
			}
			gotalarm = 0;
			sigprocmask(SIG_UNBLOCK, &mask, NULL);
			jsRead(js, &v1, &v2);
			printf("\rFrame: %5d  ", ts);
			fflush(stdout);
			if (v1!=oldv1 || v2!=oldv2) {
				vorzeSet(vorze, v1, v2);
				oldv1=v1; oldv2=v2;
				fprintf(f, "%d,%d,%d\n", ts, v1, v2);
				fflush(f);
			} else {
				vorzeDoResendIfNeeded(vorze);
			}
			ts++;
		}
		fclose(f);
		jsClose(js);
	}
}
