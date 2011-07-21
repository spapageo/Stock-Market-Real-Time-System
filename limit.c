#include "marketSim.h"
#include "limit.h"

void *limitWorker(void *arg){
	while(1){
		if ((!lsl->empty) && (!lbl->empty)) {
			if ((currentPriceX10 > lGetHead(lsl)->price1) && (currentPriceX10 < lGetHead(lbl)->price1)) {
				llPairDelete(lsl,lbl);
			}
		}
	}
	return NULL;
}


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

void llPairDelete(llist *sl, llist *bl){
	// Lock both list
	pthread_mutex_lock(sl->mut);
	pthread_mutex_lock(bl->mut);
	pthread_mutex_lock(price_mut);
	
	//Check if a pair of market orders is available
	if ((!sl->empty) && (!bl->empty)) {
		if ((currentPriceX10 > lGetHead(sl)->price1) && (currentPriceX10 < lGetHead(bl)->price1)) {
			int vol1 = sl->HEAD->ord.vol;
			int vol2 = bl->HEAD->ord.vol;
			long int id1 = sl->HEAD->ord.id;
			long int id2 = bl->HEAD->ord.id;
			order ord;
			if (vol1 < vol2) {
				vol2 = vol2 - vol1;
				llistDel(sl,&ord);
				lGetHead(bl)->vol = vol2;
				pthread_cond_signal(sl->notFull);

				currentPriceX10 = lGetHead(sl)->price1 + (lGetHead(bl)->price1 - lGetHead(sl)->price1)/2;
				fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol1, id1, id2);
				fflush(log_file);
			} else if (vol1 > vol2) {
				vol1 = vol1 - vol2;
				llistDel(bl,&ord);
				lGetHead(sl)->vol = vol1;
				pthread_cond_signal(bl->notFull);

				currentPriceX10 = lGetHead(sl)->price1 + (lGetHead(bl)->price1 - lGetHead(sl)->price1)/2;
				fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol2, id1, id2);
				fflush(log_file);
			} else {
				llistDel(sl,&ord);
				llistDel(bl,&ord);
				pthread_cond_signal(sl->notFull);
				pthread_cond_signal(bl->notFull);

				currentPriceX10 = lGetHead(sl)->price1 + (lGetHead(bl)->price1 - lGetHead(sl)->price1)/2;
				fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol1, id1, id2);
				fflush(log_file);
			}
		}
	}

	pthread_mutex_unlock(price_mut);
	pthread_mutex_unlock(sl->mut);
	pthread_mutex_unlock(bl->mut);
}


