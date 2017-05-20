/** @file gateway.c */

/*! \var int run
 * \brief Não Utilizada?
 */

/*! \var int sc
* \brief Client side socket descriptor
*/

/*! \var int sp
* \brief Server side socket descriptor
*/

/*! \var serverlist servers
* \brief Linked list of servers
*/

/*! \var int ID
* \brief Server ID counter
*/

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

/**
* \def CLIENT_SIDE_PORT
* \brief  Client side port
*/

/**
* \def PEER_SIDE_PORT
* \brief  Peer side port
*/

#define CLIENT_SIDE_PORT 3000
#define PEER_SIDE_PORT 3005

/****** GLOBAL VARIABLES ********/
int run = 1;
int sc, sp; // client side and server side socket descriptors
serverlist *servers;  // linked list of servers
int ID = 0; /*server ID counter */
uint64_t photo_id = 0; /*photos id counter
int n_peers = 0;   /* number of total peers */
/********************************/

/*! \fn void sigint_handler(int n)
    \brief Signal to close the \a gateway
    \param n PAKÉISTO?.
*/
void sigint_handler(int n){
    close(sc);
    close(sp);
    printf("Gateway terminating\n");
    exit(EXIT_SUCCESS);
}

/*! \fn check_and_update_peer(message_gw *gw_msg, pthread_mutex_t *list_key)
    \brief Attempt to connect to server and confirm
     if it is dead
    \param gw_msg
    \param list_key
*/
int check_and_update_peer(message_gw *gw_msg, pthread_rwlock_t *rwlock){
    struct sockaddr_in srv_addr;
    int s, sgw;
    int retval;
    int rmt_identifier;

    if((s = socket(AF_INET, SOCK_STREAM, 0))==-1){
		perror("socket");
		exit(EXIT_FAILURE);
	}

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(gw_msg->port);
	inet_aton(gw_msg->address, &srv_addr.sin_addr);

	if( (sgw = connect(s, (const struct sockaddr *) &srv_addr, sizeof(struct sockaddr_in))) == 0){
		printf("Server with port=%d is still alive\n", gw_msg->port);
        /* comunicate with server that gateway is just checking its status */
        rmt_identifier = 3;
        send(s, &rmt_identifier, sizeof(rmt_identifier), 0);
        close(sgw);
        close(s);
        return 0;
	}

    printf("Server with address:%s and port %d stopped working\n", gw_msg->address, gw_msg->port);
    retval = delete_peer(&servers, &n_peers, gw_msg->address, gw_msg->port, rwlock);
    if(retval == 1){
        printf("Found and successfully deleted peer\n");
        printf("Total number of peers is %d\n", n_peers);
    } else if(retval == 0){
        printf("No server to delete found in serverlist\n");
    } else if(retval == -1){
        printf("Serverlist is empty, no peers to delete\n");
    }

    close(sgw);
    close(s);
    return 0;
}


/*! \fn void *c_interact(void *list_key)
    \brief Thread that interacts with client requests, receives socket descriptor
    \param list_key
*/
void *c_interact(void *rwlock){
    socklen_t addr_len;
    struct sockaddr_in rmt_addr;
    serverlist *tmp_node;
    message_gw gw_msg;

    while(1){
		addr_len = sizeof(rmt_addr);
		recvfrom(sc, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &rmt_addr, &addr_len);
		if(gw_msg.type == 0){ //contacted by client
			printf("Contacted by new client\n");
			if( (tmp_node = pick_server(&servers, rwlock)) != NULL){
				gw_msg.type = 1; //notify server is available
				strcpy(gw_msg.address, tmp_node->address);
				gw_msg.port = tmp_node->port;

                //no need for rwlock, worst realistic case slight load imbalance
				printf("Server available with port %d\n", tmp_node->port);
				tmp_node->nclients = tmp_node->nclients + 1;

				sendto(sc, &gw_msg, sizeof(gw_msg), 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr));
                continue;
			}
			else{
				printf("No servers available\n");
				gw_msg.type = 0; // notify no server available
				sendto(sc, NULL, 0, 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr));
			}

		} else if(gw_msg.type == -1){ //server connection lost
                check_and_update_peer(&gw_msg, rwlock);
                continue;
        } else{
            printf("Received message from client with non-defined type %d\n", gw_msg.type);
        }
	}
}

/*! \fn void *p_interact(void *list_key)
    \brief Thread that interacts with peer requests, receives socket descriptor
    \param list_key
*/
void *p_interact(void *rwlock){
    socklen_t addr_len;
    int retval;
    struct sockaddr_in rmt_addr;
    serverlist *tmp_node;
    message_gw gw_msg;

    while(1){
		addr_len = sizeof(rmt_addr);
		recvfrom(sp, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &rmt_addr, &addr_len);
		if(gw_msg.type == 1){ //contacted by peer
			retval = add_server(&servers, &n_peers, inet_ntoa(rmt_addr.sin_addr), gw_msg.port, ID, rwlock);
            if(retval == 1){ /* guaranteed its the first server */
                ID = 0; //go back to ID=0 if all peers are gone
            }

			sendto(sp, &ID, sizeof(ID), 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr)); //send back ID information
			ID++;
			printf("New server available - addr=%s, port =%d\n", servers->address, servers->port);
            printf("System contains %d peers total\n", n_peers);
		} else if(gw_msg.type == 2){   //server lost 1 connection
			if(!(tmp_node = search_server(&servers, gw_msg.ID, rwlock)))
				printf("Can't find server in list\n");
			else
                pthread_rwlock_rdlock(rwlock);//no need for wrlock (worst case slight load imbalance
                if(tmp_node)//may have been deleted meanwhile
                    tmp_node->nclients = tmp_node->nclients - 1;
                pthread_rwlock_unlock(rwlock);
                printf("Updated information from server\n");
		} else if(gw_msg.type == -1){   //peer to peer connection lost
                check_and_update_peer(&gw_msg, rwlock);
        } else if (gw_msg.type == 4){   //peer querying photo_id
			sendto(sp, &photo_id, sizeof(photo_id), 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr)); //send back photo_id information
            //######## NEED photo_id GUARD !!!#####
            photo_id++;
        }
        else if(gw_msg.type ==5 ){/* peer searching for father */
            if(!(tmp_node = search_father(&servers, gw_msg.ID, rwlock))){
                printf("Peer can't be found on list, cannot attribute father peer\n");
                continue;
            }
            printf("Peer with port %d will connect to peer with port %d\n", tmp_node->next->port, tmp_node->port);
            /* if the peer searching for a father is the oldest peer
             * then we can guarantee he is updated
             * there must always be a peer 0 */
            if(searchlist_crown_head(&servers, gw_msg.ID, rwlock)){
                gw_msg.ID = 0;//comunicate that peer is new 0 peer
            }
            strcpy(gw_msg.address, tmp_node->address);
            gw_msg.port = tmp_node->port;
            sendto(sp, &gw_msg, sizeof(gw_msg), 0, (const struct sockaddr*) &rmt_addr, sizeof(rmt_addr));
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
    pthread_rwlock_t rwlock;                 //rwlock to guard serverlist

    pthread_rwlock_init(&rwlock,NULL);

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

	if((sc=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	if((bind(sc, (const struct sockaddr *) &gwc_addr, sizeof(gwc_addr))) == -1){
		perror("bind error");
		exit(EXIT_FAILURE);
	}

  if( pthread_create(&client_side, NULL, c_interact, &rwlock) != 0){
				printf("Error creating a new thread\n");
                exit(EXIT_FAILURE);
    }

/****** INITIALIZE peer side socket ******/
	gwp_addr.sin_family = AF_INET;
	gwp_addr.sin_port = htons(PEER_SIDE_PORT);
	gwp_addr.sin_addr.s_addr = INADDR_ANY;

	if((sp=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	if((bind(sp, (const struct sockaddr *) &gwp_addr, sizeof(gwp_addr))) == -1){
		perror("bind error");
		exit(EXIT_FAILURE);
	}

  if( pthread_create(&peer_side, NULL, p_interact, &rwlock) != 0){
		printf("Error creating a new thread\n");
      exit(EXIT_FAILURE);
   }

/****** END OF EXECUTION ********/
    pause(); //pause indefinately until SIGINIT
}
