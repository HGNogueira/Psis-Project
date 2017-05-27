#ifndef KEYWORDLIST_INIT_H
#define KEYWORDLIST_INIT_H

#include <pthread.h>
#include <sys/types.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "idlist.h"
#include "photolist.h"

/* prototype for keyword list node, contains all information regarding each key*/

typedef struct keyword{
    id_node *ids;
    char *keyword;
    struct keyword *next;
}keyword_node;


// INITIALIZE KEYWORDLIST_INIT TO NULL
#define KEYWORDLIST_INIT() NULL

// ADD KEYWORD TO keywordlist, TO BE INSERTED IN SORTED ORDER
// RETURN NULL IF KEYWORD ALREADY IN THE LIST
// RETURNS keyword_node * TO THE  KEYWORD INSERTED IN THE LIST
// CAN RETURN VOID!
keyword_node *keywordlist_insert(keyword_node **, char *, uint32_t, pthread_rwlock_t *, photolist_t **photos, pthread_rwlock_t *photolock);
/******************************************************************************/
// DELETE ALL keywordlist
void keywordlist_delete(keyword_node **, pthread_rwlock_t *);
/******************************************************************************/
// DELETE GIVEN ID from keywordlist -> Function used in deletion of a PHOTO
void keywordlist_remID(keyword_node *, uint32_t, pthread_rwlock_t *);
/******************************************************************************/
// PRINT ALL keywordlist
void keywordlist_print(keyword_node *, pthread_rwlock_t *);
/******************************************************************************/
// PRINT ALL keyword and respective IDS
void keywordlist_printAllData(keyword_node *h, pthread_rwlock_t *rwlock);
/******************************************************************************/
// PRINT respective IDS from a given keyword
void keywordlist_printIDS_fromKey(keyword_node *h, char *, pthread_rwlock_t *rwlock);
/******************************************************************************/
// Search for a given key
// Return NULL if no match
// Return node of the keyword found
int search_keyword(keyword_node *, char *, uint32_t **, pthread_rwlock_t *);

#endif //KEYWORDLIST_INIT_H
