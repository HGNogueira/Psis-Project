#ifndef PEERLIST_H
#define PEERLIST_H
#include <pthread.h>
#include <sys/types.h>

typedef struct node{
	int port;
	char address[50];
	int nclients;
	int ID;
    struct node *prev;
	struct node *next;
} peerlist;



peerlist *init_server();
peerlist *pick_server(peerlist **peers, pthread_rwlock_t *rwlock);//choose server according to least load rule
int add_server(peerlist **peers, int *n_peers, char *address, int port, int ID, pthread_rwlock_t *rwlock);
int delete_peer(peerlist **peers, int *n_peers, char *address, int port, pthread_rwlock_t *rwlock);
peerlist *search_server(peerlist **peers, int ID, pthread_rwlock_t *rwlock);
peerlist *search_father(peerlist **peers, int ID, pthread_rwlock_t *rwlock);
int searchlist_crown_head(peerlist **peers, int ID, pthread_rwlock_t *rwlock);

#endif
