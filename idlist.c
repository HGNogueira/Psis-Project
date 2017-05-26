#include "idlist.h"

int IDlist_insert(id_node **hp, unsigned id, pthread_rwlock_t *rwlock){
	id_node *a, *p;
	for(a = (id_node*)hp, p = *hp; p != NULL && id > p->id; a = p, p = p->next)
		;
    if(p != NULL && id == p->id)
        return 0;
    id_node  *n = malloc(sizeof *n);
	n->id = id;
	n->next = p;
	a->next = n;
    return 1;
}

void IDlist_print(id_node *h, pthread_rwlock_t *rwlock){
	id_node *p;
	pthread_rwlock_rdlock(rwlock);
	for(p = h; p != NULL; p = p->next)
		printf("(%u) ", p->id);
	pthread_rwlock_unlock(rwlock);
	putchar('\n');
}

void IDlist_delete(id_node **hp, pthread_rwlock_t *rwlock){
	id_node *p, *aux;
	for(p = *hp; p != NULL; p = aux){
		aux = p->next;
		free(p);
	}
	*hp = NULL;
}

id_node *IDlist_match(id_node *hp, unsigned id, pthread_rwlock_t *rwlock){
	while(hp != NULL)
		if(id > hp->id)
			hp = hp->next;
		else
			return id == hp->id ? hp : NULL;
	return NULL;
}

static id_node *IDlist_Specialmatch(id_node **hp, unsigned id, pthread_rwlock_t *rwlock){
	id_node *prev = (id_node *)hp;
	id_node *aux = *hp;
	for(; aux != NULL && id > aux->id; prev = aux, aux = aux->next)
		;
	if(!aux)
		return NULL;
	return id == aux->id ? prev : NULL;
}

int IDlist_del_el(id_node **hp, unsigned id, pthread_rwlock_t *rwlock){
	id_node *prev;
	if((prev = IDlist_Specialmatch(hp, id, rwlock)) != NULL){ // REMOVE ELEMENT if true
		if(!prev->next->next){ // List at the end or with only 1 element
			id_node *aux = prev->next;
			prev->next = NULL;
			free(aux);
		}
		else if(prev == (id_node *)hp){
			prev = *hp;
			(*hp) = (*hp)->next;
			free(prev);
		}
		else{
			id_node *aux = prev->next;
			prev->next = prev->next->next;
			free(aux);
		}
		return 1;
	}
	return 0;
}
