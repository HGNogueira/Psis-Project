#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>
#include "messages.h"
#define S_PORT 3001

/************************ Functional structures for server.c ******************/
    /* list of threads to easily initiate threads willingly */
struct pthread_node{
	pthread_t thread_id;
	int s;
	struct pthread_node *next;
};

    /* list of photos, includes extra information for peer replication purposes */
typedef struct photo_node{
    uint32_t photo_id;
    char photo_name[50];
    unsigned photo_size;
    int exists; //indicates if photo exists within database or still mereley information about it
    int deleted; //indicates if photo has been deleted
    struct photo_node *next;
} photolist_t;

typedef struct task_node{
    task_t task;
    struct task_node *previous;
} tasklist_t;
/******************************************************************************/


/****************** GLOBAL VARIABLES ******************************************/
int run = 1, s_gw; //s_gw socket que comunica com gateway (partilhada entre threads)
struct sockaddr_in gw_addr;
message_gw gw_msg;
int ID;            //identificador de servidor atribuido pela gateway
int updated = 0;   //identifica se peer já foi updated à cadeia de transferêcia de info
struct pthread_node *thread_list, *thread_head;  //lista de threads
tasklist_t *tasklist;
photolist_t *photolist;
/******************************************************************************/

void sigint_handler(int n){
	run = 0;
}

void convert_toupper(char *string){
	int i = 0;
	while(string[i] != '\0'){
		string[i] = toupper(string[i]);
		i++;
	}
}
    /* this thread function implements the getting up to date routine */
void *get_updated(void *thread_scl){

}

    /* this thread function implements the server updating routine */
void update_peer(void *thread_scl){

}

    /* this thread function implements normal son-peer interaction */
void pson_interact(void *thread_scl){
    int s;

    s = (int) *((int*) thread_scl);

    while(1){
        /*wait for signal */
        send(s, &(tasklist->task), sizeof(task_t), 0);
    }
}

    /* this thread function implements father-peer interaction */
void *pfather_interact(void *dummy){
    int s, fatherpeer = 2;
    int err;
    message_gw gw_msg;
    socklen_t addr_len;
    struct sockaddr_in peer_addr;
    task_t recv_task;
    tasklist_t *tmp_tasklist;
    photolist_t *tmp_photolist;

    /* wait to know who will be peer father, only thread/occasion where server recfrom gw */
	recvfrom(s_gw, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &gw_addr, &addr_len);

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(gw_msg.port);
	inet_aton(gw_msg.address, &peer_addr.sin_addr);

    if(updated = 0){
        //initiate new father peer interaction thread MISSING GUARD######
        if( pthread_create(&(thread_list->thread_id) , NULL, get_updated, &(thread_list->s)) != 0)
                    printf("Error creating a new thread\n");

        thread_list->next = (struct pthread_node *) malloc(sizeof(struct pthread_node));
    }

	connect(s, (const struct sockaddr *) &peer_addr, sizeof(struct sockaddr_in));
    send(s, &fatherpeer, sizeof(fatherpeer), 0);

    while(1){
        if( (err = recv(s, &recv_task, sizeof(recv_task), 0)) == -1){
			perror("recv error");
            close(s);
			pthread_exit(NULL);
		}
		else if(err == 0){ //peer disconnected
			printf("Client disconnected from this server\n");
			gw_msg.type = 3; //indicar à gateway que pai desconectou-se
			gw_msg.ID = ID;
			if( (sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr)) )==-1){
				perror("GW contact");
			}
			close(s);
            pfather_interact(NULL); //go back to trying to connect with new father
			return;
		}

        /* only process task if this peer isn't the one responsible for it */
        if(recv_task.ID != ID ){
            /* process new task */
            tmp_tasklist = (tasklist_t*) malloc(sizeof(tasklist_t));
            switch(recv_task.type){
                case -1:
                    printf("Deleting photo with id=%"PRIu64"\n", recv_task.photo_id);
                    tmp_tasklist->task = recv_task;

                    /* delete photo */

                    break;
                case 0:
                    printf("Adding keyword to photo with id=%"PRIu64"\n", recv_task.photo_id);
                    tmp_tasklist->task = recv_task;

                    /* add keyword to keyword list */

                    break;
                case 1:
                    printf("Adding new photo with id=%"PRIu64"\n", recv_task.photo_id);
                    tmp_tasklist->task = recv_task;
                    /* add photo to photolist */
                    tmp_photolist = (photolist_t*) malloc(sizeof(photolist_t));
                    tmp_photolist->photo_id = recv_task.photo_id;
                    strcmp(tmp_photolist->photo_name, recv_task.photo_name);
                    tmp_photolist->photo_size = recv_task.photo_size;
                    tmp_photolist->exists = 0;
                    tmp_photolist->deleted = 0;

                    /*######### NEED GUARD #######*/
                    tmp_photolist->next = photolist;
                    photolist = tmp_photolist;


                    break;
                default:
                    printf("Unknown routine for task with type %d\n", recv_task.type);
                    free(tmp_tasklist);
                    continue;
            }
        }

        /* insert tmp_list to head of task list */ /*###### NEED GUARD #### */
        tmp_tasklist->previous = tasklist;
        tasklist = tmp_tasklist; 

        /* signal pson thread to propagate this task */
    }
}

    /* this thread function implements to client-peer interaction */
void c_interact(void *thread_scl){
	int scl = (int) *((int *) thread_scl);
	int err;
	message smsg;
	message rmsg;

	while(1){
		if( (err = recv(scl, &rmsg, sizeof(message), 0)) == -1){
			perror("recv error");
			pthread_exit(NULL);
		}
		else if(err == 0){ //client disconnected
			printf("Client disconnected from this server\n");
			gw_msg.type = 2; //decrementar numero de clientes
			gw_msg.ID = ID;
			if( (sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr)) )==-1){
				perror("GW contact");
			}
			close(scl);
			return;
		}
		printf("New receive\n");
		strcpy( smsg.buffer, rmsg.buffer);
		convert_toupper(smsg.buffer);
		printf("Turned %s into %s\n", rmsg.buffer, smsg.buffer);
		if( (err = send(scl, &smsg, sizeof(smsg), 0)) == -1){
                perror("send error");
        }
	}
}

    /* id_socket identifies which type of comunication has been established */
void *id_socket(void *thread_s){
    int err;
    int rmt_identifier;
    message_gw gw_msg;
    int s = (int) *( (int *) thread_s);

    if( (err = recv(s, &rmt_identifier, sizeof(rmt_identifier), 0)) == -1){
			perror("recv error");
			pthread_exit(NULL);
    } else if(err == 0){ //client disconnected
			printf("Unidentified remote connection disconnected from this peer\n");
			close(s);
			return(NULL);
	}

    if( rmt_identifier == 0){ //approached by new client
        c_interact(thread_s);
    } else if( rmt_identifier == 1){ //peer requires updating
        update_peer(thread_s); //will start updating peer with the existing content
    } else if( rmt_identifier == 2){ //peer joins the chain
        pson_interact(thread_s);
    } 
}

int main(){
	int s, srmt, err;
	FILE *f;
	struct sockaddr_in srv_addr;
	struct sockaddr_in rmt_addr;
	socklen_t rmt_addr_len;
	char fread_buff[50];
	int port;
	struct sigaction act_INT, act_SOCK;
	socklen_t addr_len;

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


/****** PREPARE SOCK_STREAM ******/
	if(  (s = socket(AF_INET, SOCK_STREAM, 0))==-1 ){
		perror("socket");
		exit(EXIT_FAILURE);
	}
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(S_PORT + getpid());
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	
	if(  (err = bind(s, (const struct sockaddr *) &srv_addr, sizeof(struct sockaddr_in)))== -1){
		perror("bind");
		exit(EXIT_FAILURE);
	}
	
	if( (err = listen(s, 10)) == -1){
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Ready to accept\n");
/****** SOCK_STREAM PREPARED *******/
	
/****** CONTACT GATEWAY ******/
	gw_addr.sin_family = AF_INET;

	if( (f = fopen("./gw-server.txt", "r"))== NULL){
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	fgets(fread_buff, 50, f);//lê porto
	sscanf(fread_buff, "%d", &port);
	printf("Gateway port: %d\n", port);
	gw_addr.sin_port = htons(port);

	fgets(fread_buff, 50, f);//lê endereço
	printf("Gateway address: %s\n", fread_buff);
	inet_aton(fread_buff, &gw_addr.sin_addr);
	fclose(f);
	
	gw_msg.type = 1;
	gw_msg.port = S_PORT + getpid();
	printf("My port is %d\n", gw_msg.port);
	
	if( (s_gw=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	if( (sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr)) )==-1){
		perror("GW contact");
		exit(EXIT_FAILURE);
	}
	addr_len = sizeof(gw_addr);
	recvfrom(s_gw, &ID, sizeof(ID), 0, (struct sockaddr *) &gw_addr, &addr_len);
	printf("I was assign ID=%d\n", ID);

    //initialize tasklist
    tasklist = NULL;
    //initialize photolise
    photolist = NULL;

    //initiate new father peer interaction thread
    if( pthread_create(&(thread_list->thread_id) , NULL, pfather_interact, NULL) != 0)
				printf("Error creating a new thread\n");

    thread_list->next = (struct pthread_node *) malloc(sizeof(struct pthread_node));
    thread_list = thread_list->next;


    /****** READY TO RECEIVE MULTIPLE CONNECTIONS ******/
	rmt_addr_len = sizeof(struct sockaddr_in);
	thread_list = (struct pthread_node *) malloc(sizeof(struct pthread_node));
	thread_head = thread_list;
	while(run){
		if( (thread_list->s = accept(s, (struct sockaddr *) &rmt_addr, &rmt_addr_len)) != -1){

            //initiate thread to identify and proceed with interaction
			if( pthread_create(&(thread_list->thread_id) , NULL, id_socket, &(thread_list->s)) != 0)
				printf("Error creating a new thread\n");


			thread_list->next = (struct pthread_node *) malloc(sizeof(struct pthread_node));
			thread_list = thread_list->next;
		} else{
			perror("accept");
			//exit(EXIT_FAILURE):  //não sair para não interromper restantes threads

		}
	}
	
	/* close thread_list */
	
	close(s);
	exit(EXIT_SUCCESS);
}
