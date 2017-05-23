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
#include <semaphore.h>
#include "photolist.h"
#include "phototransfer.h"
#include "messages.h"
#define S_PORT 3005

/************************ Functional structures for server.c ******************/
    /* list of threads to easily initiate threads willingly */
struct pthread_node{
	pthread_t thread_id;
	int s;
	struct pthread_node *next;
};

typedef struct task_node{
    task_t task;
    struct task_node *prev;
    struct task_node *next;
} tasklist_t;

/******************************************************************************/


/****************** GLOBAL VARIABLES ******************************************/
int run = 1, s_gw; //s_gw socket que comunica com gateway (partilhada entre threads)
<<<<<<< HEAD
pthread_mutex_t gw_mutex;      
=======
pthread_mutex_t gw_mutex;
>>>>>>> lino
pthread_mutex_t thread_mutex;
pthread_mutex_t task_mutex;
pthread_rwlock_t photolock;
struct sockaddr_in gw_addr;
int ID;            //identificador de servidor atribuido pela gateway
int updated = 0;   //identifica se peer já foi updated à cadeia de transferêcia de info
struct pthread_node *thread_list, *thread_head;  //lista de threads
tasklist_t *tasklist;
photolist_t *photolist;
sem_t task_sem;
sem_t update_sem; //semaphore used to block update routine until peer becomes up to date
pthread_mutex_t update_mutex;
int pson_run = 0;
pthread_t pson_thread = 0;
int reconnected = 0; //indicates if server is in reconnected status (lost previous father connection)
/******************************************************************************/

void sigint_handler(int n){
	run = 0;
}

    /* this thread function implements the getting up to date routine
     *
     * this process starts sequentally running every task given from more recent to older */
void *get_updated(void *thread_s){
    int s = (int) *((int *) thread_s);
	int err;
    int acknowledge = 1;
    int retval;
    int photo_id;
    task_t recv_task;
    message_gw gw_msg;
    socklen_t addr_len;
    photolist_t *tmp_photolist;
    tasklist_t *tmp_tasklist, *current_node;


    pthread_mutex_lock(&task_mutex);
    if(tasklist == NULL){
        /* create dummy task, serves as a bridge between tasks to be seen as 

         * previous and after update cycle */
        current_node = (tasklist_t *) malloc(sizeof(tasklist_t));
        current_node->task.type = -3; //indicates dummy task
        current_node->prev = NULL;
        current_node->next = NULL;
        tasklist = current_node;
    }
    else
        current_node = tasklist;
    pthread_mutex_unlock(&task_mutex);

	while(1){
		if( (err = recv(s, &recv_task, sizeof(task_t), 0)) == -1){
			perror("get_update: recv error");
            //update procedure failed, shutdown server
            exit(EXIT_FAILURE);
		}
		else if(err == 0){ //client disconnected
			printf("Peer disconnected from this server while updating me...\n");
            pthread_mutex_unlock(&update_mutex);
            pthread_exit(NULL);
		}
        pthread_mutex_lock(&update_mutex); /* Can't update while getting updated/reconnected */
		printf("Received new task from Updator\n");
        tmp_tasklist = (tasklist_t *) malloc(sizeof(tasklist_t));
        /* process new task */
        switch(recv_task.type){
            case -2:
                printf("Updating process has reached its end, I am up to date\n");
                updated = 1;
                close(s);
                sem_post(&update_sem);
                pthread_mutex_unlock(&update_mutex);
                return NULL;
            case -1: //this case should never be acted with this architecure
                printf("get_updated: asked to delete photo while getting updated!\n");
                break;

            case 0:
                printf("Adding keyword to photo with id=%"PRIu64"\n", recv_task.photo_id);

                /***** MISSING IMPLEMENTATION ******/
                /* add keyword to keyword list */

                break;
            case 1:
                printf("Adding new photo with name=%s\n", recv_task.photo_name);
                /* add photo to photolist */
                retval = photolist_insert(&photolist, recv_task.photo_id, recv_task.photo_name, recv_task.photo_size, &photolock);
                if(retval == 1 && reconnected == 1){
                    /* Reconnected peer is confirmed to be back in the chain
                     * no information lost */
                    free(tmp_tasklist);
                    acknowledge = 0;
                    send(s, &acknowledge, sizeof(acknowledge), 0); //end update routine
                    close(s);
                    reconnected = 0;
                    printf("I am reconnected\n");
                    pthread_mutex_unlock(&update_mutex);
                    pthread_exit(NULL);
                }
                if(retval == 0){
                    acknowledge = 2;
                    send(s, &acknowledge, sizeof(acknowledge), 0); //end update routine

                    retval = phototransfer_recv(s, recv_task.photo_name);
                    if(retval != 0){
                        printf("get_updated: couldn't receive new photo with name %s\n", recv_task.photo_name);
                        photolist_delete(&photolist, recv_task.photo_id, recv_task.photo_size,  &photolock);
                        acknowledge = 1;
                        send(s, &acknowledge, sizeof(acknowledge), 0); //for now acknowledge is always 1

                        free(tmp_tasklist);
                        pthread_mutex_unlock(&update_mutex);
                        continue;
                    }
                }

                if(retval == 1){
                    free(tmp_tasklist);
                    acknowledge = 1;
                    send(s, &acknowledge, sizeof(acknowledge), 0);
                    pthread_mutex_unlock(&update_mutex);
                    continue;
                }

                strcpy(tmp_tasklist->task.photo_name, recv_task.photo_name);

                break;
            default:
                printf("Strange task type (%d) while getting up to date... danger of unobtained synchronization...", recv_task.type);
                send(s, &acknowledge, sizeof(acknowledge), 1); //for now acknowledge is always 1
                free(tmp_tasklist);
                pthread_mutex_unlock(&update_mutex);
                continue;
        }
        /* add task to tasklist as previous task */
        /* no danger of missing guard, only one righting opposite sense
         * can only update if it isnt getting updated */
        tmp_tasklist->task.type = recv_task.type;
        tmp_tasklist->task.ID = recv_task.ID;
        tmp_tasklist->task.photo_id = recv_task.photo_id;
        tmp_tasklist->task.photo_size = recv_task.photo_size;

        tmp_tasklist->next = current_node;
        tmp_tasklist->prev = current_node->prev;
        current_node->prev = tmp_tasklist;
        current_node = tmp_tasklist;

        send(s, &acknowledge, sizeof(acknowledge), 0); //for now acknowledge is always 1
        pthread_mutex_unlock(&update_mutex);
    }
}

    /* this thread function implements the server updating routine */
void update_peer(void *thread_s){
    int s = (int) *((int*) thread_s);
    int err, i;
    int acknowledge;
    tasklist_t *update_list, *auxlist;
    tasklist_t **deleter_list;
    int deleter_dim = 0, stop = 0;
    task_t termination_task;

    pthread_mutex_lock(&update_mutex); /* Can't update while getting updated/reconnected */
    deleter_list = NULL;
    pthread_mutex_lock(&task_mutex);
    update_list = tasklist;
    pthread_mutex_unlock(&task_mutex);

    while(1){
        /* if its the last item on tasklist */
        if(update_list == NULL){
            termination_task.type = -2; //updating process is over
            /* delete deleter tasks */
            for(i = 0; i < deleter_dim; i++){
                pthread_mutex_lock(&task_mutex);
                if(deleter_list[i] == tasklist){//ignore if its the actual tasklist pointer
                    pthread_mutex_unlock(&task_mutex);
                    break;
                }
                if(deleter_list[i]->prev != NULL)
                    (deleter_list[i]->prev)->next = deleter_list[i]->next;
                if(deleter_list[i]->next != NULL)
                    (deleter_list[i]->next)->prev = deleter_list[i]->prev;
                free(deleter_list[i]);
                pthread_mutex_unlock(&task_mutex);
            }
            /* free deleter_list array memory */
            if(deleter_list != NULL)
                free(deleter_list);
            send(s, &termination_task, sizeof(task_t), 0);
            printf("Peer is up to date\n");
            close(s);
            pthread_mutex_unlock(&update_mutex);
            return;
        }

        if(update_list->task.type == -3){ //don't send dummy task
            update_list = update_list->prev;
            continue;
        }
        if(update_list->task.type == 2){ //don't print task
            update_list = update_list->prev;

            continue;
        }
        if(update_list->task.type == -1){//no need to send delete, save info for later
            deleter_dim++;
            deleter_list = (tasklist_t**) realloc(deleter_list, sizeof(tasklist_t*)*deleter_dim);
            deleter_list[deleter_dim - 1] = update_list; //save pointer to tasklist
            update_list = update_list->prev;
            continue;
        }
        if(update_list->task.type == 0){
            /*check if photo has been deleted, no need to pass task if so */
            for(i = 0; i < deleter_dim; i++){
                if( deleter_list[i]->task.photo_id == update_list->task.photo_id){
                    pthread_mutex_lock(&task_mutex);
                    if(update_list->prev != NULL)
                        (update_list->prev)->next = update_list->next;
                    if(update_list->next != NULL)
                        (update_list->next)->prev = update_list->prev;
                    pthread_mutex_unlock(&task_mutex);

                    auxlist = update_list;
                    update_list = update_list->prev;
                    free(auxlist);
                    stop = 1;
                    break;
                }
            }
            if(stop == 1){
                stop = 0;
                continue;
            }

            if( (err = send(s, &(update_list->task), sizeof(task_t), 0) ) == -1){
                perror("send error");
            }
        }

        if(update_list->task.type == 1){
            /*check if photo has been deleted, no need to pass task if so */
            for(i = 0; i < deleter_dim; i++){
                if( deleter_list[i]->task.photo_id == update_list->task.photo_id){
                    pthread_mutex_lock(&task_mutex);
                    if(update_list->prev != NULL)
                        (update_list->prev)->next = update_list->next;
                    if(update_list->next != NULL)
                        (update_list->next)->prev = update_list->prev;
                    pthread_mutex_unlock(&task_mutex);

                    auxlist = update_list;
                    update_list = update_list->prev;
                    free(auxlist);
                    stop = 1;
                    break;
                }
            }
            if(stop == 1){
                stop = 0;
                continue;
            }
            if( (err = send(s, &(update_list->task), sizeof(task_t), 0) ) == -1){
                perror("send error");
            }
            pthread_mutex_unlock(&update_mutex);
            if(recv(s, &acknowledge, sizeof(acknowledge), 0) <= 0){
                if(deleter_list != NULL)
                    free(deleter_list);
                close(s);
                pthread_mutex_unlock(&update_mutex);
                return;
            }
            if(acknowledge == 2){//start transfering photo
                phototransfer_send(s, update_list->task.photo_name);
            }
        }

        /* wait for acknowledge */
        if(recv(s, &acknowledge, sizeof(acknowledge), 0) <= 0){
            if(deleter_list != NULL)
                free(deleter_list);
            close(s);
            pthread_mutex_unlock(&update_mutex);
            return;
        }
        sleep(1);

       if(acknowledge == 0){//no need to continue
            /* free deleter_list array memory */
            /*don't erase information since we might not have gone to the bottom */
            if(deleter_list != NULL)
                free(deleter_list);
            printf("Peer is back in the information chain\n");
            close(s);
            pthread_mutex_unlock(&update_mutex);
            return;
        }
        update_list = update_list->prev;
        pthread_mutex_unlock(&update_mutex);
    }

}

    /* this thread function implements normal son-peer interaction */
    /* it propagates the last task to its son */
void pson_interact(void *thread_s){
    int s = (int) *((int*) thread_s);
    int err;
    int acknowledge; //indicates if son peer wants next or previous item on list

    /* semaphore wait */
    sem_wait(&task_sem);
    pthread_mutex_lock(&task_mutex);
    tasklist_t *auxlist = tasklist;
    pthread_mutex_unlock(&task_mutex);

    while(pson_run){
        sleep(1);
        printf("New task for my son\n");
        send(s, &(auxlist->task), sizeof(task_t), 0);

        if(auxlist->task.type == 1){
            if(recv(s, &acknowledge, sizeof(acknowledge), 0) <= 0){
                close(s);
                return;
            }
            if(acknowledge == 2){//start transfering photo
                phototransfer_send(s, auxlist->task.photo_name);

                if(recv(s, &acknowledge, sizeof(acknowledge), 0) <= 0){
                    close(s);
                    return;
                }
                /* semaphore: wait for new tasks */
                sem_wait(&task_sem);
                auxlist = auxlist->next;
                continue;
            }
            sem_wait(&task_sem);
            auxlist = auxlist->next;
            continue;
        }

        if(recv(s, &acknowledge, sizeof(acknowledge), 0) <= 0){
            close(s);
            return;
        }
        /* semaphore: wait for new tasks */
        sem_wait(&task_sem);

>>>>>>> lino
        auxlist = auxlist->next;
    }
    printf("Closing old pson\n");
    close(s);
}

    /* this thread function implements father-peer interaction */
void *pfather_interact(void *dummy){
    int s, s_up; //socket s for father, s_up for update
    int fatherpeer = 2, updatepeer = 1; //constants to contact father and updator
    int retval;
    int err;
    message_gw gw_msg;
    socklen_t addr_len;
    struct sockaddr_in peer_addr;
    task_t recv_task;
    tasklist_t *tmp_tasklist;
    photolist_t *tmp_photolist;
    int acknowledge = 1; //used to ask for previous tasks if connection has been previously broken (recon global variable);

    /* demand gateway a new father peer */
    gw_msg.ID = ID;

    pthread_mutex_lock(&gw_mutex);
    addr_len = sizeof(gw_addr);
    sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr));
    /* wait to know who will be peer father, only thread/occasion where server recfrom gw */
	recvfrom(s_gw, &gw_msg, sizeof(gw_msg), 0, (struct sockaddr *) &gw_addr, &addr_len);
    pthread_mutex_unlock(&gw_mutex);

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(gw_msg.port);
	inet_aton(gw_msg.address, &peer_addr.sin_addr);

    if(gw_msg.ID == 0){
        ID = 0; //I was crowned the head of the list
        printf("I am the head of the list\n");
        updated = 1;
        sem_post(&update_sem);
    }

    if(updated == 0){// || reconnected ==1){
        printf("Contacting father in order to get updated\n");
        if(  (s_up = socket(AF_INET, SOCK_STREAM, 0))==-1 ){
            perror("Update socket");
            exit(EXIT_FAILURE);
        }

        if( connect(s_up, (const struct sockaddr *) &peer_addr, sizeof(struct sockaddr_in)) == -1){
            gw_msg.type = -1;
            sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr));
            perror("Update connect");
            pfather_interact(NULL); //go back to trying to connect with new father
        }
        send(s_up, &updatepeer, sizeof(updatepeer), 0);//indicate you wish to get updated

        //initiate new father peer interaction thread MISSING GUARD######

        pthread_mutex_lock(&thread_mutex);
        thread_list->s = s_up;
        if( pthread_create(&(thread_list->thread_id) , NULL, get_updated, &(thread_list->s)) != 0){
            printf("Error creating a new thread\n");
        }

        thread_list->next = (struct pthread_node *) malloc(sizeof(struct pthread_node));
        thread_list = thread_list->next;
        pthread_mutex_unlock(&thread_mutex);
    }


	if(  (s = socket(AF_INET, SOCK_STREAM, 0))==-1 ){
		perror("father socket");
		exit(EXIT_FAILURE);
	}

    if( connect(s, (const struct sockaddr *) &peer_addr, sizeof(struct sockaddr_in)) == -1){
        gw_msg.type = -1;
        sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr));
        perror("father connect");
        pfather_interact(NULL); //go back to trying to connect with new father
        exit(EXIT_FAILURE);
    }
    send(s, &fatherpeer, sizeof(fatherpeer), 0);

    printf("Connected to my father with port %d\n", gw_msg.port);

    while(1){
        if( (err = recv(s, &recv_task, sizeof(recv_task), 0)) == -1){
			perror("recv error");
            close(s);
            pfather_interact(NULL); //go back to trying to connect with new father
		}
		else if(err == 0){ //peer disconnected
			printf("Father peer disconnected from this server\n");
			gw_msg.type = -1; //indicar à gateway que pai desconectou-se
			gw_msg.ID = ID;
            reconnected = 1;  //status changes to reconnected
			if( (sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr)) )==-1){
				perror("GW contact");
			}
			close(s);
            pfather_interact(NULL); //go back to trying to connect with new father
			return NULL;            //never gets here, perpetual function
		}
        printf("New task from my father\n");

        acknowledge = 1; //in case recv_task.ID=ID I know I can go to next task
        /* only process task if this peer isn't the one responsible for it */
        if(ID == 0){//count number of loops task is taking around the list
            recv_task.turns++;
        }
        if(recv_task.ID != ID && recv_task.turns < 2){
            /* process new task */
            tmp_tasklist = (tasklist_t*) malloc(sizeof(tasklist_t));
            switch(recv_task.type){
                case -1:
                    printf("Deleting photo with id=%"PRIu64"\n", recv_task.photo_id);
                    retval = photolist_delete(&photolist, recv_task.photo_id, recv_task.photo_size, &photolock);
                    if(retval == -1){/*already deleted or non existant */
                        free(tmp_tasklist);
                        acknowledge = 1;
                        send(s, &acknowledge, sizeof(acknowledge), 0);
                        continue;
                    }

                    break;
                case 0:
                    printf("Adding keyword to photo with id=%"PRIu64"\n", recv_task.photo_id);
                    strcpy(tmp_tasklist->task.keyword, recv_task.keyword);

                    /* add keyword to keyword list */

                    break;
                case 1:
                    printf("Adding new photo with name=%s\n", recv_task.photo_name);
                    strcpy(tmp_tasklist->task.photo_name, recv_task.photo_name);
                    /* add photo to photolist */
                    retval = photolist_insert(&photolist, recv_task.photo_id, recv_task.photo_name, recv_task.photo_size, &photolock);
                    if(retval == 1){//photo already in list, stop propagation
                        free(tmp_tasklist);
                        acknowledge = 1;
                        send(s, &acknowledge, sizeof(acknowledge), 0);
                        continue;
                    }
                    if(retval == 0){
                        acknowledge = 2;
                        send(s, &acknowledge, sizeof(acknowledge), 0);

                        retval = phototransfer_recv(s, recv_task.photo_name);
                        if(retval != 0){
                            printf("get_updated: couldn't receive new photo with name %s\n", recv_task.photo_name);
                            photolist_delete(&photolist, recv_task.photo_id, recv_task.photo_size, &photolock);
                            free(tmp_tasklist);
                            acknowledge = 1;
                            send(s, &acknowledge, sizeof(acknowledge), 0);
                            continue;
                        }
                    }

                    break;
                case 2:
                    if(photolist_print(&photolist, &photolock) == -1)
                        printf("Empty list\n");
                    break;
                default:
                    printf("pfather_interact: Unknown routine for task with type %d\n", recv_task.type);
                    free(tmp_tasklist);
                    acknowledge = 0;
                    send(s, &acknowledge, sizeof(acknowledge), 0);
                    continue;
            }
            /* copy task information */
            tmp_tasklist->task.type = recv_task.type;
            tmp_tasklist->task.ID = recv_task.ID;
            tmp_tasklist->task.photo_id = recv_task.photo_id;
            tmp_tasklist->task.photo_size;
            tmp_tasklist->task.turns = recv_task.turns;

            /* insert tmp_list to head of task list */ 

            pthread_mutex_lock(&task_mutex);
            tmp_tasklist->next = NULL;
            tmp_tasklist->prev = tasklist;
            if(tasklist!=NULL){
                tasklist->next = tmp_tasklist;
            } 

            tasklist = tmp_tasklist;
            sem_post(&task_sem);
            pthread_mutex_unlock(&task_mutex);
        }
        send(s, &acknowledge, sizeof(acknowledge), 0);
    }
}

    /* this thread function implements to client-peer interaction */
void c_interact(void *thread_scl){
	int scl = (int) *((int *) thread_scl);
	int err;
    int retval;
    int photo_id;
    task_t recv_task;
    message_gw gw_msg;
    socklen_t addr_len;
    tasklist_t * tmp_tasklist;
    photolist_t *tmp_photolist;

	while(1){
		if( (err = recv(scl, &recv_task, sizeof(task_t), 0)) == -1){
			perror("recv error");
            close(scl);
			return;
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
		printf("Received new task from client\n");
        printf("ID = %d, type = %d\n", recv_task.ID, recv_task.type);

        /* process new task */
        tmp_tasklist = (tasklist_t*) malloc(sizeof(tasklist_t));
        switch(recv_task.type){
            case -1:
                printf("Deleting photo with id=%"PRIu64"\n", recv_task.photo_id);
                retval = photolist_delete(&photolist, recv_task.photo_id, recv_task.photo_size, &photolock);
                if(retval == -1){/*already deleted or non existant */
                    free(tmp_tasklist);
                    continue;
                }

                break;
            case 0:
                printf("Adding keyword to photo with id=%"PRIu64"\n", recv_task.photo_id);
                strcpy(tmp_tasklist->task.keyword, recv_task.keyword);

                /* add keyword to keyword list */

                break;
            case 1:
                printf("Adding new photo with name %s\n", recv_task.photo_name);
                strcpy(tmp_tasklist->task.photo_name, recv_task.photo_name);

                /* might be useful to check if name is occupied */

                /* ask for photo_id from gateway */
                gw_msg.type = 4;
                gw_msg.ID = ID;
                sendto(s_gw, &gw_msg, sizeof(gw_msg), 0,(const struct sockaddr *) &gw_addr, sizeof(gw_addr));

                addr_len = sizeof(gw_addr);
                recvfrom(s_gw, &photo_id, sizeof(photo_id), 0, (struct sockaddr *) &gw_addr, &addr_len);

                recv_task.photo_id = photo_id;
                /* add photo to photolist */
                retval = photolist_insert(&photolist, recv_task.photo_id, recv_task.photo_name, recv_task.photo_size, &photolock);

                retval = phototransfer_recv(scl, recv_task.photo_name);
                if(retval != 0){
                    photolist_delete(&photolist, recv_task.photo_id, recv_task.photo_size, &photolock);
                    free(tmp_tasklist);
                    continue;
                }

                break;
            case 2:
                if(photolist_print(&photolist, &photolock) == -1)
                    printf("Empty list\n");
                break;
            default:
                printf("c_interact: Unknown routine for task with type %d\n", recv_task.type);
                free(tmp_tasklist);
                continue;
        }
        /*copy task information */
        tmp_tasklist->task.type = recv_task.type;
        tmp_tasklist->task.ID = ID;
        tmp_tasklist->task.photo_id = recv_task.photo_id;
        tmp_tasklist->task.photo_size = recv_task.photo_size;
        if(ID != 0)
            tmp_tasklist->task.turns = 0;
        else
            tmp_tasklist->task.turns = 1;

        /* insert tmp_list to head of task list */
        pthread_mutex_lock(&task_mutex);
        tmp_tasklist->next = NULL;
        tmp_tasklist->prev = tasklist;
        if(tasklist!=NULL){
            tasklist->next = tmp_tasklist;
        } 
        tasklist = tmp_tasklist;
        sem_post(&task_sem);
        pthread_mutex_unlock(&task_mutex);

	}
}

    /* id_socket identifies which type of comunication has been established */
void *id_socket(void *thread_s){
    int err;
    int rmt_identifier;
    message_gw gw_msg;
    int s = (int) *( (int *) thread_s);

    if( (err = recv(s, &rmt_identifier, sizeof(rmt_identifier), 0)) == -1){
			perror("id_socket: recv error");
			pthread_exit(NULL);
    } else if(err == 0){ //client disconnected
			printf("Unidentified remote connection disconnected from this peer\n");
			close(s);
			return(NULL);
	}

    if( rmt_identifier == 0){ //approached by new client
        c_interact(thread_s);
        return NULL;
    } else if( rmt_identifier == 1){ //peer requires updating
        if(updated)
            sem_post(&update_sem);
        sem_wait(&update_sem);
        update_peer(thread_s); //will start updating peer with the existing content
        return NULL;
    } else if( rmt_identifier == 2){ //peer joins the chain
        if(pson_run != 0){
            printf("New pson, make other pson stop thread\n");
            pson_run = 0;
            sem_post(&task_sem);
            void *retval;
            pthread_join(pson_thread, &retval);
        }
        pson_thread = pthread_self();
        pson_run = 1;

        pson_interact(thread_s);
        return NULL;
    }
    if( rmt_identifier == 3){ //gateway is just checking if server is alive
        close(s);
        return NULL;
    }
}

    /* id_socket identifies which type of comunication has been established */
void *id_socket(void *thread_s){
    int err;
    int rmt_identifier;
    message_gw gw_msg;
    int s = (int) *( (int *) thread_s);

    if( (err = recv(s, &rmt_identifier, sizeof(rmt_identifier), 0)) == -1){
			perror("id_socket: recv error");
			pthread_exit(NULL);
    } else if(err == 0){ //client disconnected
			printf("Unidentified remote connection disconnected from this peer\n");
			close(s);
			return(NULL);
	}

    if( rmt_identifier == 0){ //approached by new client
        c_interact(thread_s);
        return NULL;
    } else if( rmt_identifier == 1){ //peer requires updating
        if(updated)
            sem_post(&update_sem);
        sem_wait(&update_sem);
        update_peer(thread_s); //will start updating peer with the existing content
        return NULL;
    } else if( rmt_identifier == 2){ //peer joins the chain
        if(pson_run != 0){
            printf("New pson, make other pson stop thread\n");
            pson_run = 0;
            sem_post(&task_sem);
            void *retval;
            pthread_join(pson_thread, &retval);
        }
        pson_thread = pthread_self();
        pson_run = 1;
            
        pson_interact(thread_s);
        return NULL;
    } 
    if( rmt_identifier == 3){ //gateway is just checking if server is alive
        close(s);
        return NULL;
    }
}

int main(){
	int s, srmt, err, aux_s;
	FILE *f;
	struct sockaddr_in srv_addr;
	struct sockaddr_in rmt_addr;
	socklen_t rmt_addr_len;
    message_gw gw_msg;
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

	if( (err = listen(s, 1000)) == -1){
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
    pthread_mutex_init(&task_mutex, NULL);
    pthread_mutex_init(&gw_mutex, NULL);
    sem_init(&task_sem, 0, 0); //initialize semaphore with value 0
    //initialize photolist
    photolist = photolist_init(); 
    pthread_rwlock_init(&photolock, NULL);

    pthread_mutex_init(&thread_mutex, NULL);
    pthread_mutex_lock(&thread_mutex);
	thread_list = (struct pthread_node *) malloc(sizeof(struct pthread_node));
    //initiate new father peer interaction thread
    if( pthread_create(&(thread_list->thread_id) , NULL, pfather_interact, NULL) != 0)
				printf("Error creating a new thread\n");

    thread_list->next = (struct pthread_node *) malloc(sizeof(struct pthread_node));
    thread_list = thread_list->next;
    pthread_mutex_unlock(&thread_mutex);

    sem_init(&update_sem, 0, 0); //initialize semaphore with value 0
    pthread_mutex_init(&update_mutex, NULL);
    /****** READY TO RECEIVE MULTIPLE CONNECTIONS ******/
	rmt_addr_len = sizeof(struct sockaddr_in);
	while(run){
		if( (aux_s = accept(s, (struct sockaddr *) &rmt_addr, &rmt_addr_len)) != -1){
            pthread_mutex_lock(&thread_mutex);
            thread_list->s = aux_s;

            //initiate thread to identify and proceed with interaction
			if( pthread_create(&(thread_list->thread_id) , NULL, id_socket, &(thread_list->s)) != 0)
				printf("Error creating a new thread\n");


			thread_list->next = (struct pthread_node *) malloc(sizeof(struct pthread_node));
			thread_list = thread_list->next;
            pthread_mutex_unlock(&thread_mutex);
		} else{
			perror("accept");
			//exit(EXIT_FAILURE):  //não sair para não interromper restantes threads

		}
	}

	/* close thread_list */

	close(s);
	exit(EXIT_SUCCESS);
}
