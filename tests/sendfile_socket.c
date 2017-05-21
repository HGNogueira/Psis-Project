#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MYPORT 3005

int main(){
    int s, scl;
    socklen_t addrlen;
    struct sockaddr_in srv_addr, rmt_addr;
    unsigned char *img;

    // OPEN FILE TO READ
    FILE * f = fopen("image.png", "r");

    // GET FILE SIZE AND ALLOC SUFFICIENT MEMORY
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek (f, 0, SEEK_SET);
    img = malloc(length * sizeof(unsigned char));

    // STORE DATA INTO BUFFER
    fread(img, sizeof(unsigned char), length, f);
    fclose(f);

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

    send(scl, &length, sizeof(length), 0);
    send(scl, img, sizeof(unsigned char)*length, 0);
    close(s);
}
