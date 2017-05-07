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
serverlist *pick_server(struct node *servers, pthread_mutex_t *list_key){
	struct node *s1,*s2;
	int clients;

	if(!servers)//servers vazio
		return NULL;

	s1 = servers;
	s2 = servers;
    
    pthread_mutex_lock(list_key); //danger of server stop existing
	while(s1->next){
		if(s2->nclients > s1->next->nclients){
			s2 = s1->next;
		}
		s1 = s1->next;
	}
    pthread_mutex_unlock(list_key);

	return s2;
}

void add_server(serverlist **servers, char *address, int port, int ID, pthread_mutex_t *list_key){
	serverlist *newnode;

	newnode = (serverlist *) malloc(sizeof(serverlist));
	strcpy(newnode->address, address);
	newnode->port = port;
	newnode->ID = ID;
	newnode->nclients = 0;

    pthread_mutex_lock(list_key);
	newnode->next = *servers;
	*servers = newnode;
    pthread_mutex_unlock(list_key);
}

serverlist *search_server(serverlist *servers, int ID, pthread_mutex_t *list_key){
	struct node *s;
    
    pthread_mutex_lock(list_key);
	s = servers;
	if(s == NULL)
		return NULL;
	while(s != NULL){
		if(s->ID == ID)
			return s;
		s = s->next;
	}
    pthread_mutex_unlock(list_key);
	return NULL;
}
