#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../phototransfer.h"

#define MYPORT 3006

int main(){
    int s, scl;
    socklen_t addrlen;
    struct sockaddr_in srv_addr, rmt_addr;
    unsigned char *img;
    unsigned photo_size;

    if(  (s = socket(AF_INET, SOCK_STREAM, 0))==-1 ){
		perror("socket");
		exit(EXIT_FAILURE);
	}
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(MYPORT);
	srv_addr.sin_addr.s_addr = INADDR_ANY;

	if(  bind(s, (const struct sockaddr *) &srv_addr, sizeof(struct sockaddr_in)) ){
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if( listen(s, 1000)){
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Ready to accept\n");

    addrlen = sizeof(struct sockaddr_in);
	scl = accept(s, (struct sockaddr *) &rmt_addr, &addrlen);

    if(!phototransfer_send(scl, "image.png", &photo_size))
        printf("Photo sent\n");

    close(s);
}
