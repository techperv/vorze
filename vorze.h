#ifndef VORZE_H
#define VORZE_H

void enableSimulation();
int vorzeOpen(char *port);
int vorzeSet(int handle, int v1, int v2);
int vorzeClose(int handle);
int vorzeDoResendIfNeeded(int handle);

#endif
