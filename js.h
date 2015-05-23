#ifndef JS_H
#define JS_H

void setJSParms(int jsaxis, int fwdbtn, int revbtn, int usefwdbtn);
int jsOpen(char *devname);
void jsRead(int js, int *v1, int *v2);
void jsClose(int js);

#endif
