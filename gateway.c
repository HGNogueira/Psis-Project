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
#include "messages.h"
#include "serverlist.h"

int run = 1;
void sigint_handler(int n){
	run = 0;
}

int main(){
	int s, err;
	FILE *f;
	struct sockaddr_in gw_addr;
	struct sockaddr_in rmt_addr;
	socklen_t addr_len;
	message_gw gw_msg;
	char fread_buff[50], client_send_buffer[50];
	int port, available;
	struct sigaction act_INT, act_SOCK;
	serverlist *servers, *tmp_node;
	int ID = 0; /*server ID counter */

/****** SIGNAL MANAGEMENT ******/
	act_INT.sa_handler = sigint_handler;
	sigemptyset(&act_INT.sa_mask);
	act_INT.sa_flags=0;
	sigaction(SIGINT, &act_INT, NULL);

	act_SOCK.sa_handler = sigint_handler;
	sigemptyset(&act_SOCK.sa_mask);
	act_SOCK.sa_flags=0;
	sigaction(SIGPIPE, &act_SOCK, NULL); //Quando cliente fecha scl, podemos controlar comportamento do servidor
/****** SIGNAL MANAGEMENT ******/

/****** INITIALIZE SOCKET ******/
	gw_addr.sin_family = AF_INET;

	if( (f = fopen("./gw-server.txt", "r"))== NULL){
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	fgets(fread_buff, 50, f);//lê porto
	sscanf(fread_buff, "%d", &port);
	printf("Gateway port: %d\n", port);
	gw_addr.sin_port = htons(port);
	gw_addr.sin_addr.s_addr = INADDR_ANY;
	fclose(f);
	
	if( (s=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	
	if( (bind(s, (const struct sockaddr *) &gw_addr, sizeof(gw_addr))) == -1){
		perror("bind error");
		exit(EXIT_FAILURE);
	}

/****** DGRAM SOCKET INITIALIZED ******/
	

/****** READY TO TALK TO CLIENT AND SERVER ******/
	servers = init_server();
	while(run){
		addr_len = sizeof(rmt_addr);
		recvfrom(s, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &rmt_addr, &addr_len);
		if(gw_msg.type == 1){ //contacted by peer
			add_server(&servers, inet_ntoa(rmt_addr.sin_addr), gw_msg.port, ID);
			sendto(s, &ID, sizeof(ID), 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr)); //send back ID information
			ID++;
			printf("New server available - addr=%s, port =%d\n", servers->address, servers->port);
		}
		else if(gw_msg.type == 0){ //contacted by client
			printf("Contacted by new client\n");
			if( (tmp_node = pick_server(servers)) != NULL){
				printf("Server available with port %d\n", tmp_node->port);
				gw_msg.type = 1; //notify server is available
				strcpy(gw_msg.address, tmp_node->address);
				gw_msg.port = tmp_node->port;
				tmp_node->nclients = tmp_node->nclients + 1;
				sendto(s, &gw_msg, sizeof(gw_msg), 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr));
			}
			else{
				printf("No servers available\n");
				gw_msg.type = 0; // notify no server available
				sendto(s, NULL, 0, 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr));
			}

		}
			
		else if(gw_msg.type == 3){   //server lost 1 connection
			if(!(tmp_node = search_server(servers, gw_msg.ID)))
				printf("Can't find server in list\n");
			else
				tmp_node->nclients = tmp_node->nclients - 1;
			printf("Updated information from server\n");
		}
	}
/****** END OF EXECUTION ********/
	
	printf("Closing socket\n");
	close(s);

	exit(EXIT_SUCCESS);
}
