#ifndef PHOTOTRANSFER_H
#define PHOTOTRANSFER_H
int phototransfer_send(int s, char *photo_name);
int phototransfer_recv(int s, char *photo_name);
#endif
