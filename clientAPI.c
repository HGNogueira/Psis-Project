/* Header Inclusions                                              */
#include<stdio.h>
#include<stdlib.h>
#include "clientAPI.h" /* include header with function prototypes */
#include "messages.h"  /* message description headers */

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
    struct sockaddr_in gw_addr, srv_addr;
	socklen_t size_addr;
    message_gw gw_msg;

    /****** CONTACT GATEWAY ******/
	gw_addr.sin_family = AF_INET;
	gw_addr.sin_port = port;
	inet_aton(host, &gw_addr.sin_addr);
	
	if( (s_gw=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	gw_msg.type = 0; /* type indicates client trying to establish connection */

	if( (sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(struct sockaddr *) &gw_addr, sizeof(gw_addr)) )==-1){
		perror("GW contact");
        return -1;
	}
	/****** GATEWAY CONTACT ESTABLISHED *****/

    size_addr = sizeof(struct sockaddr_in);
	recvfrom(s_gw, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &gw_addr, &size_addr);
	if(gw_msg.type == 0){
		return 0;
	}
	
	if(  (s = socket(AF_INET, SOCK_STREAM, 0))==-1 ){
		perror("socket");
		exit(EXIT_FAILURE);
	}

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(gw_msg.port);
	inet_aton(gw_msg.address, &srv_addr.sin_addr);

	if( (scl = connect(s, (const struct sockaddr *) &srv_addr, sizeof(struct sockaddr_in))) == -1){
		perror("connect");
		exit(EXIT_FAILURE);
	}
    
    return s;
}
