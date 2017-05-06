#include "serverlist.h"
#include <stdlib.h>
#include <string.h>

/* inicialização da lista de servidores */
serverlist *init_server(){
	return NULL;
}

/* função que escolhe servidor com menos carga */
serverlist *pick_server(struct node *servers){
	struct node *s1,*s2;
	int clients;

	if(!servers)//servers vazio
		return NULL;

	s1 = servers;
	s2 = servers;
	while(s1->next){
		if(s2->nclients > s1->next->nclients){
			s2 = s1->next;
		}
		s1 = s1->next;
	}

	return s2;
}

void add_server(serverlist **servers, char *address, int port, int ID){
	serverlist *newnode;

	newnode = (serverlist *) malloc(sizeof(serverlist));
	strcpy(newnode->address, address);
	newnode->port = port;
	newnode->ID = ID;
	newnode->nclients = 0;

	newnode->next = *servers;
	*servers = newnode;
}

serverlist *search_server(serverlist *servers, int ID){
	struct node *s;
	s = servers;
	if(s == NULL)
		return NULL;
	while(s != NULL){
		if(s->ID == ID)
			return s;
		s = s->next;
	}
	return NULL;
}
