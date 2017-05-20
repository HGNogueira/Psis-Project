/* Header Inclusions                                              */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

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
    int wait_time = 1, cid = 0;

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
    while(1){
        sleep(wait_time);
        if( recvfrom(s_gw, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &gw_addr, &size_addr) == -1){
            printf("Gateway not responding... waiting\n");
            wait_time ++;
            if(wait_time == 5){
                close(s_gw);
                return -1;
            }
        }
        else{
            break;
        }
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
        update_peer_loss();
		exit(EXIT_FAILURE);
	}

    //get real peer address info
    size_addr = sizeof(peer_addr);
    if( getpeername(s, (struct sockaddr *) &peer_addr, &size_addr) == -1){
        perror("getpeername");
        exit(EXIT_FAILURE);
    }

    //identify peer that I am a client
    send(s, &cid, sizeof(cid), 0);
    
    return s;
}

/*
 *  Function:
 *    gallery_add_photo
 *
 *  Description:
 *    Inserts new photo into the system.
 *
 *  Arguments:
 *    int peer_socket: value returned by connect
 *    char *file_name: name of the photo file. The photos should be read from the program directory
 *
 *  Return value:
 *    Positive integer corresponding to the photo identifier
 *    0 in case of error((Invalid file_name or problems in communication with the server).
 *
 */

uint32_t gallery_add_photo(int peer_socket, char *file_name){
    return 0;
}

/*
 *  Function:
 *    gallery_add_keyword
 *
 *  Description:
 *    Add a new keyword to a photo already in the system.
 *
 *  Arguments:
 *    int peer_socket: value returned by connect
 *    uint32_t id_photo:
 *    char *keyword:
 *
 *  Return value:
 *    Positive integer corresponding to the photo identifier
 *    0 in case of error((Invalid file_name or problems in communication with the server).
 *
 */

int gallery_add_keyword(int peer_socket, uint32_t id_photo, char *keyword){
    return 0;
}

/*
 *  Function:
 *    gallery_search_photo
 *
 *  Description:
 *    searches on the system for all photos that contain the provided keyword.
 *    The identifiers of the found photos will be stored in the array pointer by id_photos.
 *    This argument will point to an array created inside the function using calloc.
 *    The length of the id_photos array will returned by the function.
 *
 *  Arguments:
 *    int peer_socket: value returned by connect
 *    char *keyword: keyword that will be used in the search
 *    uint32_t ** id_photos: The id of the photos will be stored in an array
 *
 *  Return value:
 *    positive integer corresponding to the number of found photos
 *    0 if no photo contains the provided keyword
 *    -1 if a error occurred
 */

int gallery_search_photo(int peer_socket, char * keyword, uint32_t ** id_photos){
    return 0;
}

/*
 *  Function:
 *    gallery_delete_photo
 *
 *  Description:
 *    Removes from the system the photo identified by id_photo.
 *
 *  Arguments:
 *    int peer_socket: value returned by connect
 *    uint32_t id_photos: identifier of the photo to be removed from the system
 *
 *  Return value:
 *    1 if the photo is removed successfully
 *    0 if the photo does no exist
 *   -1 if case of error
 */

int gallery_delete_photo(int peer_socket, uint32_t id_photo){
    return 0;
}

/*
 *  Function:
 *    gallery_get_photo_name
 *
 *  Description:
 *    Retrieves from the system the name of the photo identified by id_photo
 *
 *  Arguments:
 *    int peer_socket: value returned by connect
 *    uint32_t id_pid_photo: identifier of the photo
 *    char **photo_name:
 *
 *  Return value:
 *    1 if the photo existes in the system and the name was retrieved
 *    0 if the photo does not exist
 *   -1 if case of error
 */

 int gallery_get_photo_name(int peer_socket, uint32_t id_photo, char **photo_name){
     return 0;
 }

 /*
  *  Function:
  *    gallery_get_photo
  *
  *  Description:
  *    Retrieves from the system the name of the photo identified by id_photo
  *
  *  Arguments:
  *  int peer_socket:
  *  uint32_t id_photo:
  *  char *file_name:
  *
  *  Return value:
  *    1 if the photo is downloaded successfully
  *    0 if the photo does not exist
  *   -1 if case of error
  */

 int gallery_get_photo(int peer_socket, uint32_t id_photo, char *file_name){
     return 0;
 }
