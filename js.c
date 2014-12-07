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
#include <linux/joystick.h>

int jsOpen(char *devname) {
	int js;
	js=open(devname, O_RDONLY|O_NONBLOCK);
	if (js<0) {
		perror(devname);
		exit(1);
	}
	return js;
}

void jsRead(int js, int *v1, int *v2) {
	static int a1=0, a2=0;
	long v;
	struct js_event ev;
	int l;
	do {
		l=read(js, &ev, sizeof(ev));
		if (l==sizeof(ev)) {
			if (ev.type==JS_EVENT_AXIS) {
//				printf("js: axis %d val %d\n", ev.number, ev.value);
				if (ev.number==1) a1=ev.value;
				if (ev.number==2) a2=ev.value;
			}
		}
	} while (l==sizeof(ev));
	v=a1*a2;
	v=v>>16;
	if (v<0) {
		*v1=1;
		*v2=-((float)v/80);
	} else {
		*v1=0;
		*v2=((float)v/80);
	}
	if (*v2>100) *v2=100;
}

void jsClose(int js) {
	close(js);
}