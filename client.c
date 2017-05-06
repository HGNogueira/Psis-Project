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
    int s, err;
	char scanned[MESSAGE_LEN];
	FILE *f;
	char fread_buff[50];
    int  port;
    in_port_t port_s;
    message_gw gw_msg;
	message rmsg;
	message smsg;
		
	/****** CONTACT GATEWAY ******/
    port_s = htons(GATE_PORT);
    s = gallery_connect(GATE_ADDR, port_s);
    if( s <= 0){
        exit(0);
    }
	
	while(1){  /* interaction */
		fgets(scanned, MESSAGE_LEN, stdin);
		strcpy(smsg.buffer, scanned);
		send(s, (void *) &smsg, sizeof(message), 0);
        if( (err = recv(s, &rmsg, sizeof(message), 0)) == -1){
			perror("recv error");
			pthread_exit(NULL);
		}
		else if(err == 0){ //server disconnected
            //let gateway know that server stopped responding
            /*
			gw_msg.type = 1; //comunicate server stopped responding to gateway
			if( (sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr)) )==-1){
				perror("GW contact");
			}
            */

			printf("Server stopped responding\n");
			close(s);
            exit(EXIT_FAILURE);
		}
		printf("sent %s --- received %s\n", smsg.buffer, rmsg.buffer);
	}
}
