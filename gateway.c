#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>

#include "messages.h"
#include "serverlist.h"

#define CLIENT_SIDE_PORT 3000
#define PEER_SIDE_PORT 3001

int run = 1;
int sc, sp; // client side and server side socked descriptors
serverlist *servers;  // linked list of servers, auxiliary node
int ID = 0; /*server ID counter */

void sigint_handler(int n){
    close(sc);
    close(sp);
    printf("Gateway terminating\n");
    exit(EXIT_SUCCESS);
}

/* thread that interacts with client requests, receives socket descriptor */
void *c_interact(void *dummy){
    socklen_t addr_len;
    struct sockaddr_in rmt_addr;
    serverlist *tmp_node;
    message_gw gw_msg;

    while(1){
		addr_len = sizeof(rmt_addr);
		recvfrom(sc, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &rmt_addr, &addr_len);
		if(gw_msg.type == 0){ //contacted by client
			printf("Contacted by new client\n");
			if( (tmp_node = pick_server(servers)) != NULL){
				printf("Server available with port %d\n", tmp_node->port);
				gw_msg.type = 1; //notify server is available
				strcpy(gw_msg.address, tmp_node->address);
				gw_msg.port = tmp_node->port;
				tmp_node->nclients = tmp_node->nclients + 1;
				sendto(sc, &gw_msg, sizeof(gw_msg), 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr));
			}
			else{
				printf("No servers available\n");
				gw_msg.type = 0; // notify no server available
				sendto(sc, NULL, 0, 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr));
			}

		} 
        else{
            printf("Received message from client with non-defined type %d\n", gw_msg.type);
        }
	}
}

/* thread that interacts with peer requests, receives socket descriptor */
void *p_interact(void *dummy){
    socklen_t addr_len;
    struct sockaddr_in rmt_addr;
    serverlist *tmp_node;
    message_gw gw_msg;

    while(1){
		addr_len = sizeof(rmt_addr);
		recvfrom(sp, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &rmt_addr, &addr_len);
		if(gw_msg.type == 1){ //contacted by peer
			add_server(&servers, inet_ntoa(rmt_addr.sin_addr), gw_msg.port, ID);
			sendto(sp, &ID, sizeof(ID), 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr)); //send back ID information
			ID++;
			printf("New server available - addr=%s, port =%d\n", servers->address, servers->port);
		}
		else if(gw_msg.type == 2){   //server lost 1 connection
			if(!(tmp_node = search_server(servers, gw_msg.ID)))
				printf("Can't find server in list\n");
			else
				tmp_node->nclients = tmp_node->nclients - 1;
			printf("Updated information from server\n");
		}
        else{
            printf("Received message from peer with non-defined type %d\n", gw_msg.type);
        }
	}

}

int main(){
	int err;
	struct sockaddr_in gwc_addr, gwp_addr; //client and peer side addr
	socklen_t addr_len;
	struct sigaction act_INT;
    pthread_t client_side, peer_side;

/****** SIGNAL MANAGEMENT ******/
	act_INT.sa_handler = sigint_handler;
	sigemptyset(&act_INT.sa_mask);
	act_INT.sa_flags=0;
	sigaction(SIGINT, &act_INT, NULL);
/****** SIGNAL MANAGEMENT ******/
        
	servers = init_server();  // initialize server list

/****** INITIALIZE client side socket ******/
	gwc_addr.sin_family = AF_INET;
	gwc_addr.sin_port = htons(CLIENT_SIDE_PORT);
	gwc_addr.sin_addr.s_addr = INADDR_ANY;
	
	if( (sc=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	
	if( (bind(sc, (const struct sockaddr *) &gwc_addr, sizeof(gwc_addr))) == -1){
		perror("bind error");
		exit(EXIT_FAILURE);
	}
    if( pthread_create(&peer_side, NULL, c_interact, NULL ) != 0){
				printf("Error creating a new thread\n");
                exit(EXIT_FAILURE);
    }

/****** INITIALIZE peer side socket ******/
	gwp_addr.sin_family = AF_INET;
	gwp_addr.sin_port = htons(PEER_SIDE_PORT);
	gwp_addr.sin_addr.s_addr = INADDR_ANY;
	
	if( (sp=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	
	if( (bind(sp, (const struct sockaddr *) &gwp_addr, sizeof(gwp_addr))) == -1){
		perror("bind error");
		exit(EXIT_FAILURE);
	}

    if( pthread_create(&peer_side, NULL, p_interact,NULL ) != 0){
				printf("Error creating a new thread\n");
                exit(EXIT_FAILURE);
    }
    
/****** END OF EXECUTION ********/
    pause(); //pause indefinately until SIGINIT
}
