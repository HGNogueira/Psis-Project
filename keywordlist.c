#include "keywordlist.h"

keyword_node *keywordlist_insert(keyword_node **keys, char *keyword, unsigned id, pthread_rwlock_t *rwlock){
	keyword_node *a, *p;
	for(p = *keys; p != NULL && strcoll(keyword, p->keyword) > 0; a = p, p = p->next)
		;
	if(p != NULL && !strcoll(keyword, p->keyword)){ // keyword found in list
		IDlist_insert(&p->ids, id, rwlock);
		return p;
	}
    keyword_node *n = malloc(sizeof *n);
    n->keyword = strcpy(malloc(strlen(keyword) + 1), keyword);
	n->next = p;
	if(p == *keys)
		*keys = n;
	else
		a->next = n;
    n->ids = IDLIST_INIT();
	IDlist_insert(&n->ids, id, rwlock);
	return n;
}

void keywordlist_printAllData(keyword_node *h, pthread_rwlock_t *rwlock){
	keyword_node *p;
	for(p = h; p != NULL ; p = p->next){
		printf( "(%s) ", p->keyword);
		putchar('\n');
		putchar('\t');
		IDlist_print(p->ids, rwlock);
	}
}

void keywordlist_printIDS_fromKey(keyword_node *keys, char *keyword, pthread_rwlock_t *rwlock){
	keyword_node *a, *p;
	for(p = keys; p != NULL && strcoll(keyword, p->keyword) > 0; a = p, p = p->next)
		;
	if(p != NULL && !strcoll(keyword, p->keyword)) // keyword found in list
		IDlist_print(p->ids, rwlock);
}

void keywordlist_print(keyword_node *h, pthread_rwlock_t *rwlock){
	keyword_node *p;
	for(p = h; p != NULL ; p = p->next){
		printf( "(%s) ", p->keyword);
	}
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
	for(keyword_node *p = hp; p != NULL ; p = p->next)
		IDlist_del_el(&p->ids, id, rwlock);
}
