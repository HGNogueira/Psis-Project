#include "serverlist.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

/* inicialização da lista de servidores */
serverlist *init_server(){
	return NULL;
}

/* função que escolhe servidor com menos carga */
serverlist *pick_server(serverlist **peers, pthread_rwlock_t *rwlock){
	struct node *s1,*s2;
	int clients;

    //rwlock can be improved
    pthread_rwlock_rdlock(rwlock); //node musn't be freed while copying to aux nodes
	if(!(*peers)){//servers vazio
        pthread_rwlock_unlock(rwlock);
		return NULL;
    }

	s1 = *peers;
	s2 = *peers;

	while(s1->next){
		if(s2->nclients > s1->next->nclients){
			s2 = s1->next;
		}
		s1 = s1->next;
	}
    pthread_rwlock_unlock(rwlock);

	return s2;
}

void add_server(serverlist **servers, char *address, int port, int ID, pthread_rwlock_t *rwlock){
	serverlist *newnode;

	newnode = (serverlist *) malloc(sizeof(serverlist));
	strcpy(newnode->address, address);
	newnode->port = port;
	newnode->ID = ID;
	newnode->nclients = 0;

    pthread_rwlock_wrlock(rwlock);
	newnode->next = *servers;
	*servers = newnode;
    pthread_rwlock_unlock(rwlock);
    
    return;
}

/* searches serverlist and deletes node, if not found 0 is returned, if serverlist empty -1
 * is returned, if success 1 is returned */
int delete_peer(serverlist **peers, char *address, int port, pthread_rwlock_t *rwlock){
	struct node *s1, *s2;
    
pthread_rwlock_rdlock(rwlock);
	s1 = *peers;
	if(s1 == NULL)
		return -1;
	if( strcmp(s1->address, address) == 0){
            if( s1->port == port){
                pthread_rwlock_unlock(rwlock); //transform into wrlock
                pthread_rwlock_wrlock(rwlock);
                *peers = s1->next;
                free(s1);
                pthread_rwlock_unlock(rwlock);
                return 1;
            }
    }

	while(s1->next != NULL){
		if( strcmp(s1->next->address, address) == 0){
            if( s1->port == port){
                s2 = s1->next;
                pthread_rwlock_unlock(rwlock); //transform into wrlock
                pthread_rwlock_wrlock(rwlock);
                s1->next = s2->next;
                free(s2);
                pthread_rwlock_unlock(rwlock);
                return 1;
            }

        }
		s1 = s1->next;
	}
    pthread_rwlock_unlock(rwlock);
	return 0;
}

serverlist *search_server(serverlist **servers, int ID, pthread_rwlock_t *rwlock){
	struct node *s;
  
pthread_rwlock_rdlock(rwlock);
	s = *servers;
	if(s == NULL){
        pthread_rwlock_unlock(rwlock);
		return NULL;
    }
	while(s != NULL){
		if(s->ID == ID){
            pthread_rwlock_unlock(rwlock);
			return s;
        }
		s = s->next;
	}
    pthread_rwlock_unlock(rwlock);
	return NULL;
}
