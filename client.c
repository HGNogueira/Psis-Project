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
        printf("Type task type: -1, 0, 1, 2:");
        scanf("%d", &task_type);
        task.type = task_type;
        switch(task_type){
            case -1:
                printf("You want to delete a photo, whats the ID?\n");
                scanf("%d", &photo_id);
                task.photo_id = (uint64_t) photo_id; //use task type to transf id

                /* delete photo */

                break;
            case 0:
                printf("You want to add a keyword. Type keyword:");
                scanf("%s", keyword);
                printf("Type the photo ID:");
                scanf("%d", &photo_id);
                task.photo_id = (uint64_t) photo_id; //use task type to transf id
                strcpy(task.keyword, keyword);

                /* add keyword to keyword list */

                break;
            case 1:
                printf("You want to add a photo. Whats its name?");
                scanf("%s", photo_name);
                strcpy(task.photo_name, photo_name);
                task.type = 1;

                if( send(s, (void *) &task, sizeof(task), 0) == -1){
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                phototransfer_send(s, task.photo_name);
                continue;
            case 2:
                printf("Peers will print their lists\n");
                break;
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
