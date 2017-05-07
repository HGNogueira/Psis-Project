#include <pthread.h>
#include <sys/types.h>

typedef struct node{
	int port;
	char address[50];
	int nclients;
	int ID;
	struct node *next;
} serverlist;



serverlist *init_server();
serverlist *pick_server(serverlist *, pthread_mutex_t *list_key);//choose server according to least load rule
void add_server(serverlist **servers, char *address, int port, int ID, pthread_mutex_t *list_key);
serverlist *search_server(serverlist *servers, int ID, pthread_mutex_t *list_key);

