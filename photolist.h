#include <pthread.h>
#include <sys/types.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>

    /* prototype for photo list node, contains all information regarding each 
     * photo as well as extras for replication purposes */
typedef struct photo_node_t{
    uint32_t photo_id;
    char photo_name[50];
    unsigned photo_size;
    int deleted;
    struct photo_node_t *prev;
    struct photo_node_t *next;
} photolist_t;


/* initialize photolist_t */
photolist_t *photolist_init();
/* add photo to photolist, to be inserted in sorted order according to photo_id */
int photolist_insert(photolist_t **photos, uint32_t photo_id, char *photo_name, unsigned photo_size, pthread_rwlock_t *rwlock);
int photolist_delete(photolist_t **photos, uint32_t photo_id, char *photo_name, unsigned photo_size, pthread_rwlock_t *rwlock);


void photolist_add_keyword(photolist_t **photos, char *keyword, pthread_rwlock_t *rwlock);
int photolist_search(photolist_t **photos, uint32_t photo_id, pthread_rwlock_t *rwlock);

