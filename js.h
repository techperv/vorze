#ifndef JS_H
#define JS_H

int jsOpen(char *devname);
void jsRead(int js, int *v1, int *v2);
void jsClose(int js);

#endif