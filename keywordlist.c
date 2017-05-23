#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "keywordlist.h"
#include "idlist.h"

// RETURNS NULL if keyword already in list
// RETURNS key if keyword inserted in list
keyword_node *keywordlist_insert(keyword_node **keys, char *keyword, pthread_rwlock_t *rwlock){
	keyword_node *a, *p;
	for(p = *keys; p != NULL && strcoll(keyword, p->keyword) > 0; a = p, p = p->next)
		;
	if(p != NULL && !strcoll(keyword, p->keyword))
        return NULL;
    keyword_node *n = malloc(sizeof *n);
    n->keyword = strcpy(malloc(strlen(keyword) + 1), keyword);
	n->next = p;
	if(p == *keys)
		*keys = n;
	else
		a->next = n;
    n->ids = NULL;
	return n;
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

int main(){
	keyword_node *keys = NULL;
	keyword_node *nkey;
	pthread_rwlock_t rwlock;
	if((nkey = keywordlist_insert(&keys, "animais", &rwlock)) != NULL){
		printf("%d\n", IDlist_insert(&nkey->ids, 0, &rwlock));
		printf("%d\n", IDlist_insert(&nkey->ids, 1, &rwlock));
		printf("%d\n", IDlist_insert(&nkey->ids, 345, &rwlock));
		printf("%d\n", IDlist_insert(&nkey->ids, 2, &rwlock));
		printf("%d\n", IDlist_insert(&nkey->ids, 2, &rwlock));
		printf("%d\n", IDlist_insert(&nkey->ids, 3, &rwlock));
	}
	keywordlist_print(keys, &rwlock);
	putchar('\n');
	IDlist_print(nkey->ids, &rwlock);
	IDlist_del_el(&nkey->ids, 2, &rwlock);
	IDlist_print(nkey->ids, &rwlock);
	keywordlist_delete(&keys ,&rwlock);
	return 0;
}
