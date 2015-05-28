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

static int jsaxis, fwdbtn, revbtn, usefwdbtn;

void setJSParms(int axis, int fbtn, int rbtn, int ufb) {
	jsaxis=axis;
	fwdbtn=fbtn;
	revbtn=rbtn;
	usefwdbtn=ufb;
}

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
	static int a=0;
	static int b1=0,b2=0;
	int v;
	struct js_event ev;
	int l;
	do {
		l=read(js, &ev, sizeof(ev));
		if (l==sizeof(ev)) {
			if (ev.type==JS_EVENT_AXIS) {
				if (ev.number==jsaxis) a=ev.value;
			} else if (ev.type==JS_EVENT_BUTTON) {
				if (ev.number==fwdbtn) b1=ev.value;
				if (ev.number==revbtn) b2=ev.value;
			}
		}
	} while (l==sizeof(ev));
	v=a;
	if (b2) {
		v=-v;
	} else if (!b1 && usefwdbtn) {
		v=0;
	}
	if (v<0) {
		*v1=1;
		*v2=-v*100/30000;
	} else {
		*v1=0;
		*v2=v*100/30000;
	}
	if (*v2>100) *v2=100;
}

void jsClose(int js) {
	close(js);
}
