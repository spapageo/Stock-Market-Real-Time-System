#include "marketSim.h"
#include "limit.h"

void lSafeAdd(llist *l,order ord) {
	pthread_mutex_lock(l->mut);
	while(l->full){
		printf("*** Incoming List is Full ***");
		fflush(stdout);
		pthread_cond_wait(l->notFull,l->mut);
	}
	llistAdd(l,ord,llistInsert(l,ord));
	pthread_cond_signal(l->notEmpty);
	pthread_mutex_unlock(l->mut);
	
}


void lSafeDelete(llist *l,order *ord) {
	pthread_mutex_lock(l->mut);
	while(l->empty){
		pthread_cond_wait(l->notEmpty,l->mut);
	}
	llistDel(l,ord);
	pthread_cond_signal(l->notFull);
	pthread_mutex_unlock(l->mut);
}


order* lGetHead(llist *l) {
	return &(l->HEAD->ord);
}

void llPairDelete(llist *l1, llist *l2){
	// Lock both list
	pthread_mutex_lock(l1->mut);
	pthread_mutex_lock(l2->mut);
	
	//Check if a pair of market orders is available
	if ((!l1->empty) && (!l2->empty)) { // FIX ME MEMEMEMEMEMEMEME WE NEED TO CHECK THE CURRENT PRICE AND LOCK ITS MUTEX BEFORE THAT
		int vol1 = l1->HEAD->ord.vol;
		int vol2 = l2->HEAD->ord.vol;
		order ord;
		if (vol1 < vol2) {
			vol2 = vol2 - vol1;
			llistDel(l1,&ord);
			lGetHead(l2)->vol = vol2;
			pthread_cond_signal(l1->notFull);
		} else if (vol1 > vol2) {
			vol1 = vol1 - vol2;
			llistDel(l2,&ord);
			lGetHead(l1)->vol = vol1;
			pthread_cond_signal(l2->notFull);
		} else {
			llistDel(l1,&ord);
			llistDel(l2,&ord);
			pthread_cond_signal(l1->notFull);
			pthread_cond_signal(l2->notFull);
		}
	}
	
	pthread_mutex_unlock(l1->mut);
	pthread_mutex_unlock(l2->mut);
}
