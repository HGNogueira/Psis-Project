#ifndef PHOTOLIST_H
#define PHOTOLIST_H

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
    int deleted;
    struct photo_node_t *prev;
    struct photo_node_t *next;
} photolist_t;


/* initialize photolist_t */
photolist_t *photolist_init();
/* add photo to photolist, to be inserted in sorted order according to photo_id */
int photolist_insert(photolist_t **photos, uint32_t photo_id, char *photo_name, pthread_rwlock_t *rwlock);
/* returns pointer to photo with photo_id, returns NULL if not found */
photolist_t *photolist_search(photolist_t **photos, uint32_t photo_id, pthread_rwlock_t *rwlock);
/* returns name of photo with photo_id through pointer, returns 1 on success */
int photolist_getname(photolist_t **photos, uint32_t photo_id, char *photo_name, pthread_rwlock_t *rwlock);
/* uploads photo to peer */
int photolist_upload(photolist_t **photos, int socket, uint32_t photo_id, pthread_rwlock_t *rwlock);
/* deletes photo from photolist and from the peer's folder */
int photolist_delete(photolist_t **photos, uint32_t photo_id, pthread_rwlock_t *rwlock);
/* prints photolist id-name pairs */
int photolist_print(photolist_t **photos, pthread_rwlock_t *rwlock);
#endif //PHOTOLIST_H
