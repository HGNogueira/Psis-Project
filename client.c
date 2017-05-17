#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "messages.h"
#include "clientAPI.h"

#define GATE_PORT 3000
#define GATE_ADDR "127.0.0.1"

int main(){
    int s, err, s_gw;
	char scanned[MESSAGE_LEN];
	FILE *f;
    int  port;
    socklen_t addrlen;
    struct sockaddr_in peer_addr, gw_addr;
    char address[50];
    in_port_t port_s;
    message_gw gw_msg;
	message rmsg;
	message smsg;

	/****** CONTACT GATEWAY ******/
    port_s = htons(GATE_PORT);
    s = gallery_connect(GATE_ADDR, port_s);
    if( s == 0){
        printf("No server available\n");
        exit(0);
    }
    if( s == -1){
        printf("Gateway can not be accessed\n");
        exit(0);
    }



	while(1){  /* interaction */
		fgets(scanned, MESSAGE_LEN, stdin);
		strcpy(smsg.buffer, scanned);
		if( send(s, (void *) &smsg, sizeof(message), 0) == -1){
            perror("send");
            exit(EXIT_FAILURE);
        }
        if( (err = recv(s, &rmsg, sizeof(message), 0)) == -1){
			perror("recv error");
			exit(EXIT_FAILURE);
		}
		else if(err == 0){ //server disconnected
            update_peer_loss();
            printf("Server stopped responding\n");
			close(s);
            exit(EXIT_FAILURE);
		}
		printf("sent %s --- received %s\n", smsg.buffer, rmsg.buffer);
	}
}
