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
#include "phototransfer.h"

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
    uint32_t *id_photos;
    int arraysize;
    int i;

    message_gw gw_msg;

    task_t task;
    int task_type;
    char photo_name[50];
    char keyword[50];
    int photo_id;
		
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
        printf("Type task type: -1, 0, 1, 2, 3, 4:");
        scanf("%d", &task_type);
        task.type = task_type;
        switch(task_type){
            case -1:
                printf("You want to delete a photo, whats the ID?\n");
                scanf("%d", &photo_id);
                gallery_delete_photo(s, photo_id);

                continue;
            case 0:
                printf("You want to add a keyword. Type keyword:");
                scanf("%s", keyword);
                printf("Type the photo ID:");
                scanf("%u", &photo_id);
                gallery_add_keyword(s, photo_id, keyword);
                /* add keyword to keyword list */

                continue;
            case 1:
                printf("You want to add a photo. Whats its name?");
                scanf("%s", photo_name);
                strcpy(task.photo_name, photo_name);
                gallery_add_photo(s, photo_name);
                continue;
            case 2:
                printf("Peers will print their lists\n");
                break;
            case 3:
                printf("Peers will print their keyword lists\n");
                break;
            case 4:
                printf("You wish to search a photo, type keyword:");
                scanf("%s", keyword);

                arraysize = gallery_search_photo(s, keyword, &id_photos);
                for(i = 0; i < arraysize; i++){
                    printf("[%"PRIu32"] ", id_photos[i]);
                }
                printf("\n");
                free(id_photos);

                continue;
            default:
                printf("Unknown routine for task with type %d\n", task_type);
                continue;
        }
		if( send(s, (void *) &task, sizeof(task), 0) == -1){
            perror("send");
            exit(EXIT_FAILURE);
        }
	}
}
