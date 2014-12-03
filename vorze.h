#ifndef VORZE_H
#define VORZE_H

int vorzeOpen(char *port);
int vorzeSet(int handle, int v1, int v2);
int vorzeClose(int handle);

#endif