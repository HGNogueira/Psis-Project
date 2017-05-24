#ifndef PHOTOTRANSFER_H
#define PHOTOTRANSFER_H
#include <inttypes.h>
int phototransfer_send(int s, char *photo_name, uint64_t photo_id);
int phototransfer_recv(int s, char *photo_name, uint64_t photo_id);
#endif
