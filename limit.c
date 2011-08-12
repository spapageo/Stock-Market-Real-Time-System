#include "marketSim.h"
#include "limit.h"
#include "market.h"

// *********************************************************
queue *lsq;
queue *lbq;

// *********************************************************
void *limitWorker(void *arg){
	char none;

	while (1) {
		none = 0;

		if ((lbq->empty == 0) && (msq->empty == 0)){
 			if(lbq->item[lbq->head].price1 >= currentPriceX10){
 				mlPairDelete( msq, lbq );
 				none=1;
 			}
 		}

 		if ((mbq->empty == 0 ) && (lsq->empty == 0)){
			if(lsq->item[lsq->head].price1 <= currentPriceX10){
 				mlPairDelete( mbq, lsq );
 				none=1;
 			}
 		}

		if((lsq->empty == 0) && (lbq->empty == 0) && (none == 0)){
			if ((currentPriceX10 >= lsq->item[lsq->head].price1) && (currentPriceX10 <= lbq->item[lbq->head].price1)) {
				llPairDelete(lsq,lbq);
			}
		}
	}
	return arg;
}

void llPairDelete(queue *sl, queue *bl){

	/* Lock both queues */
	pthread_mutex_lock(sl->mut);
	pthread_mutex_lock(bl->mut);
	pthread_mutex_lock(price_mut);

	if (!(sl->empty) && !(bl->empty)) {
		if ((currentPriceX10 >= sl->item[sl->head].price1) && (currentPriceX10 <= bl->item[bl->head].price1)) {
			int vol1 = sl->item[sl->head].vol, vol2 = bl->item[bl->head].vol;
			int pvol = 0;
			int id1 = sl->item[sl->head].id, id2 = bl->item[bl->head].id;
			order ord;
			if (vol1 < vol2) {
				currentPriceX10 = ( sl->item[sl->head].price1 + bl->item[bl->head].price1)/2;
				vol2 = vol2 - vol1;
				pvol = vol1;
				queueDel(sl,&ord);
				bl->item[bl->head].vol = vol2;
				
				pthread_cond_broadcast(sl->notFull);
			} else if (vol1 > vol2) {
				currentPriceX10 = ( sl->item[sl->head].price1 + bl->item[bl->head].price1)/2;
				vol1 = vol1 - vol2;
				pvol = vol2;
				queueDel(bl,&ord);
				sl->item[sl->head].vol = vol1;
				pthread_cond_broadcast(bl->notFull);
			} else {
				currentPriceX10 = ( sl->item[sl->head].price1 + bl->item[bl->head].price1)/2;
				queueDel(sl,&ord);
				queueDel(bl,&ord);
				pvol = vol1;
				pthread_cond_broadcast(sl->notFull);
				pthread_cond_broadcast(bl->notFull);
			}
			fprintf(log_file,"%08ld	%08ld	%5.1f	%05d	%08d	%08d\n", ord.timestamp, getTimestamp(), (float)currentPriceX10/10, pvol, id1, id2);
			fflush(log_file);
		}

	}

	pthread_mutex_unlock(price_mut);
	pthread_mutex_unlock(sl->mut);
	pthread_mutex_unlock(bl->mut);
}