#include "serverlist.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>

/* Implemented functions for serverlist looped linked list */

/* inicialização da lista de servidores */
serverlist *init_server(){
	return NULL;
}

    /* return 1 if its the first server */
int add_server(serverlist **peers, int *n_peers, char *address, int port, int ID, pthread_rwlock_t *rwlock){
	serverlist *newnode, *auxnode;
    int retval = 0;

	newnode = (serverlist *) malloc(sizeof(serverlist));
	strcpy(newnode->address, address);
	newnode->port = port;
	newnode->ID = ID;
	newnode->nclients = 0;

    pthread_rwlock_wrlock(rwlock); //must lock whole region, DANGER
    if(*peers == NULL){
        newnode->prev = newnode; //previous node is itself
        newnode->next = newnode; //next node is itself
        newnode->ID = 0; //first peer is always ID = 0
        *peers = newnode;
    } else{ //insert before list head
        auxnode = (*peers)->prev;
        newnode->prev = auxnode;
        auxnode->next = newnode;
        newnode->next = (*peers);
        (*peers)->prev = newnode;
    }//*server keeps pointing to the same node
    (*n_peers)++;
    if((*n_peers) == 1)/* first server on the list */
        retval = 1;
    pthread_rwlock_unlock(rwlock);
    
    return retval;
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
int delete_peer(serverlist **peers, int *n_peers, char *address, int port, pthread_rwlock_t *rwlock){
	struct node *s1, *auxnode;
    int startID;
    
    pthread_rwlock_wrlock(rwlock);
	s1 = *peers;
	if(s1 == NULL)
		return -1;
    if(s1->next == s1){/* list with only one element */
		if( strcmp(s1->address, address) == 0){
            if( s1->port == port){
                free(s1);
                *peers = NULL;
                (*n_peers) = 0;
                pthread_rwlock_unlock(rwlock);
                return 1;
            }
        }
    }

    startID = s1->ID;

	while(1){
		if( strcmp(s1->address, address) == 0){
            if( s1->port == port){
                if(s1 == *peers){  
                    *peers = s1->next;
                }
                (s1->prev)->next = s1->next;
                (s1->next)->prev = s1->prev;
                free(s1);
                (*n_peers)--;
                pthread_rwlock_unlock(rwlock);
                return 1;
            }

        }

        if(s1->next->ID == startID) //looped list has been searched
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
            s = s->prev;
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

int searchlist_crown_head(serverlist **peers, int ID, pthread_rwlock_t *rwlock){
    struct node *s;
    int startID;
    
    pthread_rwlock_wrlock(rwlock); //must lock whole region, sensitive!
	s = *peers;
	if(s == NULL){//list is empty
        printf("searchlist_crown_head: list is empty\n");
        pthread_rwlock_unlock(rwlock);
		return -1;
    }

    if((*peers)->ID == ID){
        (*peers)->ID = 0;
        pthread_rwlock_unlock(rwlock);
        return 1;
    }
    pthread_rwlock_unlock(rwlock);
    return 0;
}
