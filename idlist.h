#ifndef ID_LIST_H
#define ID_LIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct lNode{
	struct lNode *next;
	unsigned id;
}id_node;

/******************************************************************************/
// INITIALIZE LIST TO NULL
#define IDLIST_INIT() NULL
/******************************************************************************/
// INSERT ID IN SORTED LINKED LIST
// RETURN 0: IF ALREADY IN THE LIST
// RETRUN 1: IF SUCCESSFULY INSERTED IN THE LIST
int IDlist_insert(id_node **, unsigned, pthread_rwlock_t *);
/******************************************************************************/
// DELETE LINKED LIST
void IDlist_delete(id_node **, pthread_rwlock_t *);
/******************************************************************************/
// FIND A GIVEN ID IN THE SORTED LINKED LIST
// RETURN id_node * TO THE NODE THAT CONTAINS THE GIVEN ID
// RETURN NULL IF THE LIST DOES NOT CONTAINT THE GIVEN ID
id_node *IDlist_match(id_node *, unsigned, pthread_rwlock_t *);
/******************************************************************************/
// FUNCTION USED BY IDlist_del_el FUNCTION TO REMOVE A GIVEN ID FROM THE LIST
// RETURN NULL IF THE LIST DOES NOT CONTAINT THE GIVEN ID
// RETURN id_node * TO THE PREVIOUS NODE THAT CONTAINS THE GIVEN ID
static id_node *IDlist_Specialmatch(id_node **, unsigned, pthread_rwlock_t *);
/******************************************************************************/
// DELETE THE NODE FROM THE SORTED LIST THAT CONTAINS A GIVE ID
// RETURN 0: IF ID NOT PRESENT IN THE LIST
// RETURN 1: IF ELEMENT SUCCESSFULY REMOVED FROM THE LIST
int IDlist_del_el(id_node **, unsigned, pthread_rwlock_t *);
/******************************************************************************/
// PRINT SORTED LINKED LIST
void IDlist_print(id_node *, pthread_rwlock_t *);

#endif //ID_LIST_H
