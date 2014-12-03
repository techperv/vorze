#ifndef MPLAYERUDP_H
#define MPLAYERUDP_H

int mplayerUdpOpen(int port);
int mplayerUdpGetTimestamp(int sock);
int mplayerUdpClose(int sock);

#endif