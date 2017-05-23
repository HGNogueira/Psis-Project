#include <pthread.h>
#include <sys/types.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>
#include "idlist.h"

/* prototype for keyword list node, contains all information regarding each key*/

typedef struct keyword{
    id_node *ids;
    char *keyword;
    struct keyword *next;
}keyword_node;


/* initialize keywordlist */
keyword_node *keywordlist_init();
/* add keyword to keywordlist, to be inserted in sorted order*/
keyword_node *keywordlist_insert(keyword_node **, char *, pthread_rwlock_t *);
void keywordlist_delete(keyword_node **, pthread_rwlock_t *);
void keywordlist_print(keyword_node *, pthread_rwlock_t *);
