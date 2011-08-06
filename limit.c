#include "marketSim.h"
#include "limit.h"
#include "market.h"
//*********************************************************
llist *lsl;
llist *lbl;

//*********************************************************


void *limitWorker(void *arg){
	int none=0;
	order o1,o2;
	o1.price1 = 100;
	o2.price1 = 100;
	int switch1,switch2;
	while (1) {
		none = 0;
		switch1 = lGetHead(lbl,&o1);
		switch2 = lGetHead(lsl,&o2);
		
 		if (switch1 == 1 && !msq->empty){
 			if(o1.price1 >= currentPriceX10){
 				qlPairDelete( msq, lbl );
 				none=1;
 			}
 		}

 		if (!mbq->empty && switch2 == 1){
 			if(o2.price1 <= currentPriceX10){
 				qlPairDelete( mbq, lsl );
 				none=1;
 			}
 		}
		
		if((switch2 == 1) && (switch1 == 1) && none == 0){
			if ((currentPriceX10 >= o2.price1) && (currentPriceX10 <= o1.price1)) {
				llPairDelete(lsl,lbl);
			}
		}
	}
	return NULL;
}


void lSafeAdd(llist *l,order ord) {
	/* Lock the list mutex */
	pthread_mutex_lock(l->mut);

	/* Check if the list is full. If so, wait on the notFull condition variable */
	while(l->full){
		printf("*** Incoming List is Full ***");fflush(stdout);
		pthread_cond_wait(l->notFull,l->mut);
	}

	/* Find where to insert the new order, and perform the insertion */
	order_t *o = llistInsertHere(l,ord);
	llistAdd(l,ord,o);
	pthread_cond_broadcast(l->notEmpty);

	/* Unlock the list mutex and return */
	pthread_mutex_unlock(l->mut);
}


void lSafeDelete(llist *l,order *ord) {
	pthread_mutex_lock(l->mut);
	while(l->empty){
		pthread_cond_wait(l->notEmpty,l->mut);
	}
	llistDel(l,ord);
	pthread_cond_broadcast(l->notFull);
	
	pthread_mutex_unlock(l->mut);
}


int lGetHead(llist *l, order *o) {
	if(l->HEAD == NULL){
		return -1;
	} else {
		*o = l->HEAD->ord;
		return 1;
	}
}

void llPairDelete(llist *sl, llist *bl){

	// Lock both list
	pthread_mutex_lock(sl->mut);
	pthread_mutex_lock(bl->mut);
	pthread_mutex_lock(price_mut);
	
	if (!(sl->empty) && !(bl->empty)) {
		if ((currentPriceX10 >= sl->HEAD->ord.price1) && (currentPriceX10 <= bl->HEAD->ord.price1)) {
			int vol1 = sl->HEAD->ord.vol, vol2 = bl->HEAD->ord.vol;
			long int id1 = sl->HEAD->ord.id, id2 = bl->HEAD->ord.id;
			order ord;
			if (vol1 < vol2) {
				currentPriceX10 = sl->HEAD->ord.price1 + (bl->HEAD->ord.price1 - sl->HEAD->ord.price1)/2;
				vol2 = vol2 - vol1;
				llistDel(sl,&ord);
				bl->HEAD->ord.vol = vol2;
				pthread_cond_broadcast(sl->notFull);
				

				fprintf(log_file,"%08ld	%08ld	%5.1f	%05d	%08ld	%08ld\n", ord.timestamp, getTimestamp(), (float)currentPriceX10/10, vol1, id1, id2);
				fflush(log_file);
			} else if (vol1 > vol2) {
				currentPriceX10 = sl->HEAD->ord.price1 + (bl->HEAD->ord.price1 - sl->HEAD->ord.price1)/2;
				vol1 = vol1 - vol2;
				llistDel(bl,&ord);
				sl->HEAD->ord.vol = vol1;
				pthread_cond_broadcast(bl->notFull);
				

				fprintf(log_file,"%08ld	%08ld	%5.1f	%05d	%08ld	%08ld\n", ord.timestamp, getTimestamp(), (float)currentPriceX10/10, vol2, id1, id2);
				fflush(log_file);
			} else {
				currentPriceX10 = sl->HEAD->ord.price1 + (bl->HEAD->ord.price1 - sl->HEAD->ord.price1)/2;
				llistDel(sl,&ord);
				llistDel(bl,&ord);
				pthread_cond_broadcast(sl->notFull);
				pthread_cond_broadcast(bl->notFull);
				

				fprintf(log_file,"%08ld	%08ld	%5.1f	%05d	%08ld	%08ld\n", ord.timestamp, getTimestamp(), (float) currentPriceX10/10, vol1, id1, id2);
				fflush(log_file);
			}
		}
		
	}
	
	pthread_mutex_unlock(price_mut);
	pthread_mutex_unlock(sl->mut);
	pthread_mutex_unlock(bl->mut);
}


