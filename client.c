#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "messages.h"

int main(){
    int s;
	char scanned[MESSAGE_LEN];
	FILE *f;
	char fread_buff[50];
    int  port;
    in_port_t port_s;
	message rmsg;
	message smsg;
		
	/****** CONTACT GATEWAY ******/

	if( (f = fopen("./ifconfig.txt", "r"))== NULL){
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	fgets(fread_buff, 50, f);//lê porto
	sscanf(fread_buff, "%d", &port);
	printf("Gateway port: %d\n", port);
	fgets(fread_buff, 50, f);//lê endereço
	printf("Gateway address: %s\n", fread_buff);
	fclose(f);

    port_s = htons(port);
    s = gallery_connect(fread_buff, port_s);
    if( s <= 0){
        exit(0);
    }
	
	while(1){  /* interaction */
		fgets(scanned, MESSAGE_LEN, stdin);
		strcpy(smsg.buffer, scanned);
		send(s, (void *) &smsg, sizeof(message), 0);
		recv(s, &rmsg, sizeof(rmsg), 0);
		printf("sent %s --- received %s\n", smsg.buffer, rmsg.buffer);
	}
}
