/* Header Inclusions                                              */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "clientAPI.h" /* include header with function prototypes */
#include "messages.h"  /* message description headers */

struct sockaddr_in peer_addr, gw_addr;

/*
 *  Function:
 *    update_peer_loss();
 *
 *  Description:
 *    contacts gateway communcating that peer stopped answering
 *
 *  Arguments:
 *      void
 *
 *  Return value:
 *      void
 *    
 */
void update_peer_loss(){
    int s_gw;
    message_gw gw_msg;

    strcpy(gw_msg.address, inet_ntoa(peer_addr.sin_addr));
    gw_msg.type = -1; //comunicate server stopped responding to gateway
    gw_msg.port = ntohs(peer_addr.sin_port);

    if( (s_gw=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    if( (sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr)) )==-1){
        perror("GW contact");
    }
    close(s_gw);
    return;
}



/*
 *  Function:
 *    gallery_connect
 *
 *  Description:
 *    connects client to one Peer. Must be called before attempting other 
 *    client-server interactions.
 *
 *  Arguments:
 *    char *host: gateway address string
 *    in_port_t port: gateway port
 *
 *  Return value:
 *    socket descriptor in case of success.
 *    -1 if gateway cannot be accessed
 *    0 if no servers are available
 *    
 */

int gallery_connect(char *host, in_port_t port){
    int s_gw, s, scl;
	socklen_t size_addr;
    message_gw gw_msg;

    /****** CONTACT GATEWAY ******/
	gw_addr.sin_family = AF_INET;
	gw_addr.sin_port = port;
	inet_aton(host, &gw_addr.sin_addr);
	
	if( (s_gw=socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) == -1){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	gw_msg.type = 0; /* type indicates client trying to establish connection */

	if( (sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(struct sockaddr *) &gw_addr, sizeof(gw_addr)) )==-1){
		perror("GW contact");
	}
	/****** GATEWAY CONTACT ESTABLISHED *****/

    size_addr = sizeof(struct sockaddr_in);
    sleep(1);
	if( recvfrom(s_gw, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &gw_addr, &size_addr) == -1){
        close(s_gw);
        return -1;
    }
    close(s_gw);

	if(gw_msg.type == 0){
		return 0;
	}
	
	if(  (s = socket(AF_INET, SOCK_STREAM, 0))==-1 ){
		perror("socket");
		exit(EXIT_FAILURE);
	}

	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(gw_msg.port);
	inet_aton(gw_msg.address, &peer_addr.sin_addr);

	if( connect(s, (const struct sockaddr *) &peer_addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		exit(EXIT_FAILURE);
	}

    //get real peer address info
    size_addr = sizeof(peer_addr);
    if( getpeername(s, (struct sockaddr *) &peer_addr, &size_addr) == -1){
        perror("getpeername");
        exit(EXIT_FAILURE);
    }
    
    return s;
}
