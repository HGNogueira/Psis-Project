#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "photolist.h"
#include "phototransfer.h"

photolist_t *photolist_init(){
    return NULL;
}

    /* returns 0 if success, 1 if photo already there */
int photolist_insert(photolist_t **photos, uint32_t photo_id, char *photo_name, pthread_rwlock_t *rwlock){
    photolist_t *auxphoto, *searchnode;
    int retval;

    /* copy information to new node */
    auxphoto = (photolist_t *) malloc(sizeof(photolist_t));
    auxphoto->photo_id = photo_id;
    strcpy(auxphoto->photo_name, photo_name);
    auxphoto->next = NULL;
    auxphoto->prev = NULL;

    // must be wrlock to avoid some other thread writing first node simultaneously 

    pthread_rwlock_wrlock(rwlock);
    if(*photos == NULL){
        *photos = auxphoto;
        pthread_rwlock_unlock(rwlock);
        return 0;
    }
    searchnode = *photos;
    if(searchnode->photo_id > photo_id){//if new photo is to be inserted at start
        auxphoto->prev = searchnode->prev;
        auxphoto->next = searchnode;
        searchnode->prev = auxphoto;
        *photos = auxphoto; //start of list becomes this new photo
        pthread_rwlock_unlock(rwlock);
        return 0;
    }
    pthread_rwlock_unlock(rwlock);

    pthread_rwlock_rdlock(rwlock);
    while(searchnode->next != NULL){
        if(searchnode->photo_id == photo_id){ //ID already in the list
            pthread_rwlock_unlock(rwlock);
            return 1;
        }
        if(searchnode->photo_id > photo_id){ //passed photo_id, insert in previous
            pthread_rwlock_unlock(rwlock); //go from rwlock to wrlock
            // something might happen here, will check again
            pthread_rwlock_wrlock(rwlock);
            if(searchnode == NULL){//someone deleted this photo meanwhile
                pthread_rwlock_unlock(rwlock);
                free(auxphoto);
                auxphoto = NULL;
                //try again
                retval = photolist_insert(photos, photo_id, photo_name, rwlock);
                return retval;

            }
            while(1){
                if(searchnode != NULL){
                    if(searchnode->photo_id < photo_id){
                        auxphoto->prev = searchnode;
                        auxphoto->next = searchnode->next;
                        (searchnode->next)->prev = auxphoto;
                        searchnode->next = auxphoto;
                        //photo correctly inserted between ID's
                        pthread_rwlock_unlock(rwlock);
                        return 0;
                    }
                    searchnode = searchnode->prev;
                }else{
                    searchnode = *photos; //searchnode = list start
                    auxphoto->next = searchnode;
                    searchnode->prev = auxphoto;
                    *photos = auxphoto;
                    pthread_rwlock_unlock(rwlock);
                    return 0;
                }
            }
        }
        searchnode = searchnode->next;
    }
    //searchnode->next == NULL, at last node
    pthread_rwlock_unlock(rwlock);
    //something can happen here, repeat process starting from searchnode (faster)
    //WRITE LOCK
    pthread_rwlock_wrlock(rwlock);
    if(searchnode == NULL){//someone deleted this photo meanwhile
        pthread_rwlock_unlock(rwlock);
        free(auxphoto);
        auxphoto = NULL;
        //try again
        retval = photolist_insert(photos, photo_id, photo_name, rwlock);
        return retval;

    }
    while(searchnode->next != NULL){
        if(searchnode->photo_id == photo_id){
            pthread_rwlock_unlock(rwlock);
            //other thread already inserted it
            return 1;
        }
        if(searchnode->photo_id > photo_id){ //passed photo_id, insert in previous
            auxphoto->prev = searchnode->prev;
            auxphoto->next = searchnode;
            searchnode->prev = auxphoto;
            //photo correctly inserted between ID's
            pthread_rwlock_unlock(rwlock);
            return 0;
        }
        searchnode = searchnode->next;
    }
    if(searchnode->photo_id == photo_id){
            pthread_rwlock_unlock(rwlock);
            return 1;
    }
    if(searchnode->photo_id > photo_id){//insert in previous
        auxphoto->prev = searchnode->prev;
        auxphoto->next = searchnode;
        searchnode->prev = auxphoto;
        //photo correctly inserted between ID's
        pthread_rwlock_unlock(rwlock);
        return 0;
    }else{ //insert in next
        auxphoto->prev = searchnode;
        auxphoto->next = searchnode->next;
        searchnode->next = auxphoto;
        //photo correctly inserted at the end
        pthread_rwlock_unlock(rwlock);
        return 0;
    }
}

/* returns pointer to photo, if it doesn't exist, returns NULL */
photolist_t *photolist_search(photolist_t **photos, uint32_t photo_id, pthread_rwlock_t *rwlock){
    photolist_t *searchnode;

    /* lock outside searchloop, critical region is relatively small */
    pthread_rwlock_rdlock(rwlock);
    searchnode = *photos;
    if(searchnode == NULL){
        pthread_rwlock_unlock(rwlock);
        return NULL;
    }
    while(searchnode != NULL){
        if(searchnode->photo_id == photo_id){
            pthread_rwlock_unlock(rwlock);
            return searchnode;
        }
        searchnode = searchnode->next;
    }
    pthread_rwlock_unlock(rwlock);
    return NULL;
}


int photolist_getname(photolist_t **photos, uint32_t photo_id, char *photo_name, pthread_rwlock_t *rwlock){
    photolist_t *searchnode;

    searchnode = photolist_search(photos, photo_id, rwlock);
    pthread_rwlock_rdlock(rwlock);
    if(searchnode == NULL){
        (photo_name)[0] = '\0';
        pthread_rwlock_unlock(rwlock);
        return 0;
    }
    strcpy(photo_name, searchnode->photo_name);
    pthread_rwlock_unlock(rwlock);

    return 1;
}

/* uploads photo through socket connection, returns 1 on success, else returns 0 */
int photolist_upload(photolist_t **photos, int socket, uint32_t photo_id, pthread_rwlock_t *rwlock){
    photolist_t *toupload;
    char photo_name[50];
    int retval;
    ssize_t dummy = 0;

    toupload = photolist_search(photos, photo_id, rwlock);

    pthread_rwlock_rdlock(rwlock); 
    if(toupload == NULL){
        pthread_rwlock_unlock(rwlock);
        printf("Photo with id =%"PRIu32" no longer exists in database\n", photo_id);
        send(socket, &dummy, sizeof(dummy), 0); //send size = 0 to stop transfer
        return 0;
    }
    strcpy(photo_name, toupload->photo_name);
    pthread_rwlock_unlock(rwlock);

    retval =  phototransfer_send(socket, photo_name, photo_id);

    if(retval != 0){
        return 1;
    }
    return 0;
}

    /* returns 1 if success, 0 if can't find photo, -1 if error */
int photolist_delete(photolist_t **photos, uint32_t photo_id, pthread_rwlock_t *rwlock){
    photolist_t *searchnode;
    int retval;
    char filename[60];

    filename[0] = '\0';
    sprintf(filename, "./%" PRIu32"", photo_id);

    pthread_rwlock_rdlock(rwlock);
    if(*photos == NULL){
        pthread_rwlock_unlock(rwlock);
        return 0;
    }

    searchnode = *photos;
    if(searchnode->photo_id == photo_id){ //first in the list
            pthread_rwlock_unlock(rwlock);
            pthread_rwlock_wrlock(rwlock);
            if(searchnode == NULL){
                pthread_rwlock_unlock(rwlock);
                return 0;
            }

            if(searchnode->next != NULL){
                (searchnode->next)->prev = searchnode->prev;
            }

            strcat(filename, searchnode->photo_name);
            if(unlink(filename) == -1){
                perror("photolist_delete (unlink)");
                *photos = searchnode->next;
                free(searchnode);
                searchnode = NULL;
                pthread_rwlock_unlock(rwlock);
                return -1;
            }

            *photos = searchnode->next;
            free(searchnode);
            searchnode = NULL;
            pthread_rwlock_unlock(rwlock);
            return 1;
    }
    searchnode = searchnode->next;
    while(searchnode != NULL){
        if(searchnode->photo_id == photo_id){ //found photo_id
            pthread_rwlock_unlock(rwlock);
            pthread_rwlock_wrlock(rwlock);
            if(searchnode == NULL){
                pthread_rwlock_unlock(rwlock);
                return 0;
            }

            if(searchnode->prev != NULL){
                (searchnode->prev)->next = searchnode->next;
            }
            if(searchnode->next != NULL){
                (searchnode->next)->prev = searchnode->prev;
            }
            strcat(filename, searchnode->photo_name);
            if(unlink(filename) == -1){
                perror("photolist_delete (unlink)");
                free(searchnode);
                searchnode = NULL;
                return -1;
            }
            free(searchnode);
            searchnode = NULL;

            pthread_rwlock_unlock(rwlock);
            return 1;
        }
        searchnode = searchnode->next;
    }
    pthread_rwlock_unlock(rwlock);
    return 0;
}

int photolist_print(photolist_t **photos, pthread_rwlock_t *rwlock){
    photolist_t *searchnode;

    pthread_rwlock_rdlock(rwlock);

    if(*photos == NULL){
        pthread_rwlock_unlock(rwlock);
        return -1;
    }

    searchnode = *photos;
    while(searchnode != NULL){
        printf("%s(%"PRIu32")->", searchnode->photo_name, searchnode->photo_id);
        searchnode = searchnode->next;
    }
    pthread_rwlock_unlock(rwlock);
    printf("NULL\n");
    return 0;
}
