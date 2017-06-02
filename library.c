/* Header Inclusions                                              */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "phototransfer.h"
#include "API.h" /* include header with function prototypes */
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
    task_t task;
    uint32_t photo_id;

    task.type = 1; //type = 1 means add photo
    strcpy(task.photo_name, file_name);

    /* apply for adding a new photo */
    send(peer_socket, (void *) &task, sizeof(task), 0);

    /* call phototransfer_send function to send photo
     * -1 as arguments indicates client is calling (vs peer) */
    if(phototransfer_send(peer_socket, task.photo_name, 0) != 0){
        printf("Failed sending the photo %s\n", task.photo_name);
    }

    if(recv(peer_socket, &photo_id, sizeof(photo_id), 0) <= 0){
        printf("galery_add_photo: Peer connection failure, exiting\n");
        update_peer_loss();
        exit(EXIT_FAILURE);
    }
    if(photo_id != 0)
        printf("Successfully uploaded photo %s\n", file_name);

    return photo_id;
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
    task_t task;
    int acknowledge;

    task.type = 0; //type = 0 means add keyword
    task.photo_id = id_photo;
    strcpy(task.keyword, keyword);

    /* apply for adding a new keyword */
    send(peer_socket, (void *) &task, sizeof(task), 0);

    if(recv(peer_socket, &acknowledge, sizeof(int), 0) <= 0){
        printf("galery_add_keyword: Peer connection failure, exiting\n");
        update_peer_loss();
        exit(EXIT_FAILURE);
    }
    printf("Successfully added the new keyword\n");

    //pode-se implementar return dependendo se photo existe ou não
    //não é pedido no enunciado
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
    task_t task;
    int size, len, pointer, remain_data;

    task.type = 4;
    strcpy(task.keyword, keyword);


    /* apply for id search */
    send(peer_socket, (void *) &task, sizeof(task), 0);

    if(recv(peer_socket, &size, sizeof(int), 0) <= 0){
        printf("galery_search_photo: Peer connection failure, exiting\n");
        update_peer_loss();
        exit(EXIT_FAILURE);
    }
    if(size == 0){
        return 0;
    }

    *id_photos = (int *) malloc(sizeof(int)*size);
    remain_data = size;
    pointer = 0;
    while( (remain_data > 0) && ((len = recv(peer_socket, &((*id_photos)[pointer]), sizeof(uint32_t)*remain_data, 0)) > 0)){
        remain_data = remain_data - (int)((double)len)*((double)(sizeof(char))/((double)sizeof(uint32_t)));
        pointer = size - remain_data;
    }

    if(remain_data != 0){
        printf("galery_search_photo: remain_data = %d  , data transfer imperfect\n", remain_data);
        return -1;
    }

    return size;
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
    task_t task;
    int acknowledge;

    task.type = -1; //type = -1 means delete photo
    task.photo_id = id_photo;

    /* apply for deleting a photo */
    send(peer_socket, (void *) &task, sizeof(task), 0);

    if(recv(peer_socket, &acknowledge, sizeof(int), 0) <= 0){
        printf("galery_add_keyword: Peer connection failure, exiting\n");
        update_peer_loss();
        exit(EXIT_FAILURE);
    }
    if(acknowledge == 1){
        printf("Successfully deleted photo\n");
        return 1;
    }
    if(acknowledge == 0){
        printf("Photo not found\n");
        return 0;
    }else{
        printf("Error while deleting photo\n");
    }
    return -1;
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
    task_t task;
    int acknowledge;

    task.type = 5; //type = 5 means return name of the photo with photo_id
    task.photo_id = id_photo;

    /* apply for photo_name */
    send(peer_socket, (void *) &task, sizeof(task), 0);

    *photo_name = (char *) malloc(sizeof(photo_name)*50);

    if(recv(peer_socket, *photo_name , sizeof(char)*50, 0) <= 0){
        printf("galery_get_photo_name: Peer connection failure, exiting\n");
        update_peer_loss();
        exit(EXIT_FAILURE);
    }
    if( (*photo_name)[0] == '\0')
        return 0;

     return 1;
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
    task_t task;
    int retval;

    task.type = 6; //type = 6 means return name of the photo with photo_id
    task.photo_id = id_photo;

    /* apply for photo upload */
    send(peer_socket, (void *) &task, sizeof(task), 0);

    retval = phototransfer_recv(peer_socket, file_name, 0);
    if(retval == 1)
        return 0;
    else if(retval == -1)
        return -1;

    return 1;
 }
