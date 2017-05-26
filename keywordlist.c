#include "keywordlist.h"
#include <unistd.h>

keyword_node *keywordlist_insert(keyword_node **keys, char *keyword, unsigned id, pthread_rwlock_t *rwlock){
	keyword_node *a, *p;
	pthread_rwlock_rdlock(rwlock);
	for(p = *keys; p != NULL && strcoll(keyword, p->keyword) > 0; a = p, p = p->next)
		;
	if(p != NULL && !strcoll(keyword, p->keyword)){ // keyword found in list
		id_node *aID, *pID;
		for(aID = (id_node*)&p->ids, pID = p->ids; pID != NULL && id > pID->id; aID = pID, pID = pID->next)
			;
		if(pID != NULL && id == pID->id){ // IF TRUE -> Keyword already in linked list
			pthread_rwlock_unlock(rwlock);
			return NULL;
		}
		pthread_rwlock_unlock(rwlock);
		id_node *n = malloc(sizeof *n);
		n->id = id;
		pthread_rwlock_wrlock(rwlock);
		n->next = pID; // Assign new element to next node
		aID->next = n; // Connect new element to previous node
		pthread_rwlock_unlock(rwlock);
		return p;
	}
	pthread_rwlock_unlock(rwlock);
	keyword_node *n = malloc(sizeof *n);
    n->keyword = strcpy(malloc(strlen(keyword) + 1), keyword);
	pthread_rwlock_wrlock(rwlock);
	n->next = p;
	if(p == *keys)
		*keys = n;
	else
		a->next = n;
	n->ids = malloc(sizeof n->ids);
	n->ids->id = id;
	n->ids->next = NULL;
	pthread_rwlock_unlock(rwlock);
	return n;
}

void keywordlist_printAllData(keyword_node *h, pthread_rwlock_t *rwlock){
	keyword_node *p;
	pthread_rwlock_rdlock(rwlock);
	for(p = h; p != NULL ; p = p->next){
		printf( "(%s) ", p->keyword);
		putchar('\n');
		putchar('\t');
		id_node *pID;
		for(pID = p->ids; pID != NULL; pID = pID->next)
			printf("(%u) ", pID->id);
		putchar('\n');
	}
	pthread_rwlock_unlock(rwlock);
}

void keywordlist_printIDS_fromKey(keyword_node *keys, char *keyword, pthread_rwlock_t *rwlock){
	keyword_node *a, *p;
	pthread_rwlock_rdlock(rwlock);
	for(p = keys; p != NULL && strcoll(keyword, p->keyword) > 0; a = p, p = p->next)
		;
	pthread_rwlock_unlock(rwlock);
	if(p != NULL && !strcoll(keyword, p->keyword)) // keyword found in list
		IDlist_print(p->ids, rwlock);
}

void keywordlist_print(keyword_node *h, pthread_rwlock_t *rwlock){
	keyword_node *p;
	pthread_rwlock_rdlock(rwlock);
	for(p = h; p != NULL ; p = p->next)
		printf( "(%s) ", p->keyword);
	pthread_rwlock_unlock(rwlock);
	putchar('\n');
}

void keywordlist_delete(keyword_node **hp, pthread_rwlock_t *rwlock){
	keyword_node * p, *aux;
	for(p = *hp; p != NULL; p = aux){
		IDlist_delete(&p->ids, rwlock);
		aux = p->next;
		free(p->keyword);
		free(p);
	}
	*hp = NULL;
}

void keywordlist_remID(keyword_node *hp, unsigned id, pthread_rwlock_t *rwlock){
	keyword_node *p = hp;
	while(1){
		pthread_rwlock_rdlock(rwlock);
		if(p == NULL){
			pthread_rwlock_unlock(rwlock);
			return;
		}
		id_node *prev = (id_node *)&p->ids;
		id_node *aux = p->ids;
		for(; aux != NULL && id > aux->id; prev = aux, aux = aux->next)
			;
		if(aux != NULL && id == aux->id){
			pthread_rwlock_unlock(rwlock);
			pthread_rwlock_wrlock(rwlock);
			if(!prev->next->next){ // List at the end or with only 1 element
				id_node *aux = prev->next;
				prev->next = NULL;
				free(aux);
			}
			else if(prev == (id_node *)&p->ids){
				prev = p->ids;
				p->ids = p->ids->next;
				free(prev);
			}
			else{
				id_node *aux = prev->next;
				prev->next = prev->next->next;
				free(aux);
			}
			pthread_rwlock_unlock(rwlock);
		}
		else
			pthread_rwlock_unlock(rwlock);
		pthread_rwlock_rdlock(rwlock);
		p = p->next;
		pthread_rwlock_unlock(rwlock);
	}
}
