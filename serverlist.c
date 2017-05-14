#include "serverlist.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

/* Implemented functions for serverlist looped linked list */

/* inicialização da lista de servidores */
serverlist *init_server(){
	return NULL;
}


void add_server(serverlist **peers, char *address, int port, int ID, pthread_rwlock_t *rwlock){
	serverlist *newnode, *auxnode;

	newnode = (serverlist *) malloc(sizeof(serverlist));
	strcpy(newnode->address, address);
	newnode->port = port;
	newnode->ID = ID;
	newnode->nclients = 0;

    pthread_rwlock_wrlock(rwlock); //must lock whole region, DANGER
    if(*peers == NULL){
        newnode->prev = newnode; //previous node is itself
        newnode->next = newnode; //next node is itself
        *peers = newnode;
    } else{
        auxnode = (*peers)->next;
        newnode->next = auxnode;
        auxnode->prev = newnode;
        newnode->prev = (*peers);
        (*peers)->next = newnode;
    }//*server keeps pointing to the same node
    pthread_rwlock_unlock(rwlock);

    
    return;
}

/* função que escolhe servidor com menos carga */
serverlist *pick_server(serverlist **peers, pthread_rwlock_t *rwlock){
	struct node *s1,*s2;
	int clients;
    int startID;

    //rwlock can be improved
    pthread_rwlock_rdlock(rwlock); //node musn't be freed while copying to aux nodes
	if(!(*peers)){//servers vazio
        pthread_rwlock_unlock(rwlock);
		return NULL;
    }

    startID = (*peers)->ID;
	s1 = *peers;
	s2 = *peers;
	while(s1->next->ID != startID){
		if(s2->nclients > s1->next->nclients){
			s2 = s1->next;
		}
		s1 = s1->next;
	}
    pthread_rwlock_unlock(rwlock);

	return s2;
}

/* searches serverlist and deletes node, if not found 0 is returned, if serverlist empty -1
 * is returned, if success 1 is returned */
int delete_peer(serverlist **peers, char *address, int port, pthread_rwlock_t *rwlock){
	struct node *s1, *auxnode;
    int startID;
    
    pthread_rwlock_wrlock(rwlock);
	s1 = *peers;
	if(s1 == NULL)
		return -1;
    if(s1->next == s1){/* list with only one element */
        free(s1);
        *peers = NULL;
        return 1;
    }

    startID = s1->ID;

	while(1){
		if( strcmp(s1->address, address) == 0){
            if( s1->port == port){
                (s1->prev)->next = s1->next;
                (s1->next)->prev = s1->prev;
                free(s1);
                pthread_rwlock_unlock(rwlock);
                return 1;
            }

        }
        if(s1->next->ID == startID)
            break;
		s1 = s1->next;
	}
    pthread_rwlock_unlock(rwlock);
	return 0;
}

serverlist *search_server(serverlist **peers, int ID, pthread_rwlock_t *rwlock){
	struct node *s;
    int startID;
    
    pthread_rwlock_rdlock(rwlock);
	s = *peers;
	if(s == NULL){
        pthread_rwlock_unlock(rwlock);
		return NULL;
    }

    startID = s->ID;
	while(1){
		if(s->ID == ID){
            pthread_rwlock_unlock(rwlock);
			return s;
        }
        if(s->next->ID ==startID)
            break;
		s = s->next;
	}
    pthread_rwlock_unlock(rwlock);
	return NULL;
}

/* to be implemented ####.... */
serverlist *search_father(serverlist **peers, int ID, pthread_rwlock_t *rwlock){
	struct node *s;
    int startID;
    
    pthread_rwlock_rdlock(rwlock);
	s = *peers;
	if(s == NULL){
        pthread_rwlock_unlock(rwlock);
		return NULL;
    }

    startID = s->ID;
	while(1){
		if(s->ID == ID){
            pthread_rwlock_unlock(rwlock);
			return s->prev;
        }
        if(s->next->ID ==startID)
            break;
		s = s->next;
	}
    pthread_rwlock_unlock(rwlock);
	return NULL;
}
