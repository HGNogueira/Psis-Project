#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../phototransfer.h"

#define RMTPORT 3006
#define ADDRESS "127.0.0.1"

int main(){
    int s, scl;
    socklen_t addrlen;
    struct sockaddr_in rmt_addr;
    FILE *f;
    unsigned char *img;
    unsigned photo_size;

	rmt_addr.sin_family = AF_INET;
	rmt_addr.sin_port = htons(RMTPORT);
	inet_aton(ADDRESS, &rmt_addr.sin_addr);

    if(  (s = socket(AF_INET, SOCK_STREAM, 0))==-1 ){
		perror("socket");
		exit(EXIT_FAILURE);
	}

    connect(s, (const struct sockaddr *) &rmt_addr, sizeof(struct sockaddr_in));

    phototransfer_recv(s,"recv_photo.png", &photo_size);
    printf("Transfer complete\n");
    close(s);

    return 0;
}
