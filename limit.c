#include "marketSim.h"
#include "limit.h"
#include "market.h"

// *********************************************************
queue *lsl;
queue *lbl;

// *********************************************************


void *limitWorker(void *arg){
	char none=0;
	order o1,o2;

	while (1) {
		none = 0;

		if ((lbl->empty == 0) && (msq->empty == 0)){
 			if(lbl->item[lbl->head].price1 >= currentPriceX10){
 				qlPairDelete( msq, lbl );
 				none=1;
 			}
 		}

 		if ((mbq->empty == 0 ) && (lsl->empty == 0)){
			if(lsl->item[lsl->head].price1 <= currentPriceX10){
 				qlPairDelete( mbq, lsl );
 				none=1;
 			}
 		}

		if((lsl->empty == 0) && (lbl->empty == 0) && none == 0){
			if ((currentPriceX10 >= o2.price1) && (currentPriceX10 <= o1.price1)) {
				llPairDelete(lsl,lbl);
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
			long int id1 = sl->item[sl->head].id, id2 = bl->item[bl->head].id;
			order ord;
			if (vol1 < vol2) {
				currentPriceX10 = ( sl->item[sl->head].price1 + bl->item[bl->head].price1)/2;
				vol2 = vol2 - vol1;
				queueDel(sl,&ord);
				bl->item[bl->head].vol = vol2;
				
				pthread_cond_broadcast(sl->notFull);


				fprintf(log_file,"%08ld	%08ld	%5.1f	%05d	%08ld	%08ld\n", ord.timestamp, getTimestamp(), (float)currentPriceX10/10, vol1, id1, id2);
				fflush(log_file);
			} else if (vol1 > vol2) {
				currentPriceX10 = ( sl->item[sl->head].price1 + bl->item[bl->head].price1)/2;
				vol1 = vol1 - vol2;
				queueDel(bl,&ord);
				sl->item[sl->head].vol = vol1;
				pthread_cond_broadcast(bl->notFull);


				fprintf(log_file,"%08ld	%08ld	%5.1f	%05d	%08ld	%08ld\n", ord.timestamp, getTimestamp(), (float)currentPriceX10/10, vol2, id1, id2);
				fflush(log_file);
			} else {
				currentPriceX10 = ( sl->item[sl->head].price1 + bl->item[bl->head].price1)/2;
				queueDel(sl,&ord);
				queueDel(bl,&ord);
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