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

int main(int argc, char *argv[]){
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
    char *dyn_name;
    int photo_id;

	/****** CONTACT GATEWAY ******/
    port_s = htons(GATE_PORT);
    s = gallery_connect(GATE_ADDR, port_s);
    if(s == 0){
        printf("No server available\n");
        exit(0);
    }
    if(s == -1){
        printf("Gateway can not be accessed\n");
        exit(0);
    }

    if((f = fopen(argv[1], "r")) == NULL){
        printf("Parametros errados\n");
        exit(1);
    }
    int aux;
	while(fscanf(f, "%s", photo_name) != -1){  /* interaction */
        task.type = 1;
        strcpy(task.photo_name, photo_name);
        if(gallery_add_photo(s, photo_name))
            continue;
	}
    fclose(f);
}
