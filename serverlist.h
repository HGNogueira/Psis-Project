typedef struct node{
	int port;
	char address[50];
	int nclients;
	int ID;
	struct node *next;
} serverlist;



serverlist *init_server();
serverlist *pick_server(serverlist *);//choose server according to least load rule
void add_server(serverlist **servers, char *address, int port, int ID);
serverlist *search_server(serverlist *servers, int ID);

