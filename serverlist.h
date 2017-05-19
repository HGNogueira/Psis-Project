#include <pthread.h>
#include <sys/types.h>

typedef struct node{
	int port;
	char address[50];
	int nclients;
	int ID;
    struct node *prev;
	struct node *next;
} serverlist;



serverlist *init_server();
serverlist *pick_server(serverlist **peers, pthread_rwlock_t *rwlock);//choose server according to least load rule
int add_server(serverlist **peers, int *n_peers, char *address, int port, int ID, pthread_rwlock_t *rwlock);
int delete_peer(serverlist **peers, int *n_peers, char *address, int port, pthread_rwlock_t *rwlock);
serverlist *search_server(serverlist **peers, int ID, pthread_rwlock_t *rwlock);
serverlist *search_father(serverlist **peers, int ID, pthread_rwlock_t *rwlock);
int searchlist_crown_head(serverlist **peers, int ID, pthread_rwlock_t *rwlock);

