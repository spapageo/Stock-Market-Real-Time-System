#include "marketSim.h"
#include "limit.h"
#include <stdio.h>

void lSafeAdd(llist *l,order ord) {
	pthread_mutex_lock(l->mut);
	while(l->full){
		printf("*** Incoming List is Full ***");
		fflush(stdout);
		pthread_cond_wait(l->notFull,l->mut);
	}
	llistAdd(l,ord,llistInsert(l,ord));
	pthread_mutex_unlock(l->mut);
	pthread_cond_signal(l->notEmpty);
}


void lSafeDelete(llist *l,order *ord) {
	pthread_mutex_lock(l->mut);
	while(l->empty){
		printf("*** Incoming List is Empty ***");
		fflush(stdout);
		pthread_cond_wait(l->notEmpty,l->mut);
	}
	llistDel(l,ord);
	pthread_mutex_unlock(l->mut);
	pthread_cond_signal(l->notFull);
}


order* lGetHead(llist *l) {
	return NULL;
}
