#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "messages.h"

#define GATE_PORT 3000
#define GATE_ADDR "127.0.0.1"

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
			printf("Server stopped responding\n");
			gw_msg.type = 1; //comunicate server stopped responding to gateway
            
			gw_msg.ID = ID;
			if( (sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr)) )==-1){
				perror("GW contact");
			}
			close(scl);
			return(NULL);
		}
		printf("sent %s --- received %s\n", smsg.buffer, rmsg.buffer);
	}
}
