#include "keywordlist.h"
#include <unistd.h>

keyword_node *keywordlist_insert(keyword_node **keys, char *keyword, uint32_t id, pthread_rwlock_t *rwlock, photolist_t **photos, pthread_rwlock_t *photolock){
	keyword_node *a, *p;
	pthread_rwlock_rdlock(rwlock);
	for(p = *keys; p != NULL && strcoll(keyword, p->keyword) > 0; a = p, p = p->next)
		;
	keyword_node *read_a = a, *read_p = p; // Before leaving read, store adresses
	if(p != NULL && !strcoll(keyword, p->keyword)){ // keyword found in list
		id_node *aID, *pID;
		for(aID = (id_node*)&p->ids, pID = p->ids; pID != NULL && id > pID->id; aID = pID, pID = pID->next)
			;
		if(pID != NULL && id == pID->id){ // IF TRUE -> Keyword already in linked list
			pthread_rwlock_unlock(rwlock);
			return NULL;
		}
		id_node *read_aID = aID, *read_pID = pID; // Before leaving read, store adresses
		pthread_rwlock_unlock(rwlock);

		id_node *n = malloc(sizeof *n);
		n->id = id;

		pthread_rwlock_wrlock(rwlock);
		if(read_aID != aID || read_pID != pID){ // Something changed, traverse list again
				pthread_rwlock_unlock(rwlock);
				return keywordlist_insert(&p, keyword, id, rwlock, photos, photolock);
		}
		if(photolist_search(photos, id, photolock) != NULL){//check if id exists
            n->next = pID; // Assign new element to next node
            aID->next = n; // Connect new element to previous node
            pthread_rwlock_unlock(rwlock);
            return p;
        }
        free(n);
        pthread_rwlock_unlock(rwlock);
		return NULL;
	}
	pthread_rwlock_unlock(rwlock);

	// IF KEYWORD NOT FOUND IN LIST, INSERT KEYWORD AND ID

	pthread_rwlock_wrlock(rwlock);
	if(read_a != a || read_p != p){ // Something changed, traverse list again
			pthread_rwlock_unlock(rwlock);
			return keywordlist_insert(keys, keyword, id, rwlock, photos, photolock);
	}
    keyword_node *n = malloc(sizeof *n);
    n->keyword = strcpy(malloc(strlen(keyword) + 1), keyword);
    n->next = p;
    if(photolist_search(photos, id, photolock) != NULL){//check if id exists
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
    pthread_rwlock_unlock(rwlock);
    free(n->keyword);
    free(n);
    return NULL;
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

void keywordlist_remID(keyword_node *hp, uint32_t id, pthread_rwlock_t *rwlock){
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
		id_node *read_prev = prev, *read_aux = aux; // Before leaving read, store adresses
		if(aux != NULL && id == aux->id){
			pthread_rwlock_unlock(rwlock);
			pthread_rwlock_wrlock(rwlock);
<<<<<<< HEAD
			if(read_prev != prev || read_aux != aux){ // TEST IF ADDRESSES ARE THE SAME, IF NOT TRAVERSE AGAIN
				pthread_rwlock_unlock(rwlock);
				keywordlist_remID(p, id, rwlock);
			}
=======
			if(read_prev != prev || read_aux != aux) // TEST IF ADDRESSES ARE THE SAME, IF NOT TRAVERSE AGAIN
				keywordlist_remID(p, id, rwlock);
>>>>>>> 1b1a4e52703d62d86b7fcf3aeae2d99577ad1101
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
		else // IF ID NOT FOUND IN KEYWORD, GO TO NEXT KEYWORD
			pthread_rwlock_unlock(rwlock);
		pthread_rwlock_rdlock(rwlock);
		p = p->next;
		pthread_rwlock_unlock(rwlock);
	}
}

int search_keyword(keyword_node *keys, char *keyword, uint32_t **id_photos, pthread_rwlock_t *rwlock){
	keyword_node *p;
    id_node *ids;
	pthread_rwlock_rdlock(rwlock);
    int arraysize = 0, i = 0;

	for(p = keys; p != 0 && strcoll(keyword, p->keyword) > 0; p = p->next)
		;
	if(p == NULL){
		pthread_rwlock_unlock(rwlock);
		return 0;
	}
	else if(!strcoll(keyword, p->keyword)){
        ids = p->ids;
        while(ids != 0){
            arraysize ++;
            ids = ids->next;
        }

        *id_photos = (uint32_t *) malloc(sizeof(uint32_t)*arraysize);
        ids = p->ids;
        while(ids != 0){
            (*id_photos)[i] = ids->id;
            i++;
            ids = ids->next;
        }
		pthread_rwlock_unlock(rwlock);
		return arraysize;
	}
	else{
		pthread_rwlock_unlock(rwlock);
		return 0;
	}
}
