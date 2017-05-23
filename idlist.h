#ifndef ID_LIST_H
#define ID_LIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct lNode{
	struct lNode *next;
	unsigned id;
}id_node;

int IDlist_insert(id_node **, unsigned, pthread_rwlock_t *);
void IDlist_delete(id_node **, pthread_rwlock_t *);
id_node *IDlist_match(id_node *, unsigned, pthread_rwlock_t *);
id_node *IDlist_Specialmatch(id_node **, unsigned, pthread_rwlock_t *);
void IDlist_del_el(id_node **, unsigned, pthread_rwlock_t *);
void IDlist_print(id_node *, pthread_rwlock_t *);

#endif
