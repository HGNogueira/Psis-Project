#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define RMTPORT 3005
#define ADDRESS "127.0.0.1"

int main(){
    int s, scl;
    socklen_t addrlen;
    struct sockaddr_in rmt_addr;
    FILE *f;
    unsigned char *img;
    long length;

	rmt_addr.sin_family = AF_INET;
	rmt_addr.sin_port = htons(RMTPORT);
	inet_aton(ADDRESS, &rmt_addr.sin_addr);

    if(  (s = socket(AF_INET, SOCK_STREAM, 0))==-1 ){
		perror("socket");
		exit(EXIT_FAILURE);
	}

    connect(s, (const struct sockaddr *) &rmt_addr, sizeof(struct sockaddr_in));

    recv(s, &(length), sizeof(length), 0);

    img = (unsigned char*) malloc(sizeof(unsigned char)*length);
    
    recv(s, img, sizeof(unsigned char)*length, 0);
    // CREATE PNG FROM DATA
    f = fopen("recv.png", "w");
    fwrite(img, sizeof(char), length, f);
    fclose(f);
}
