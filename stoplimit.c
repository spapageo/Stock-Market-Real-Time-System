#include "marketSim.h"
#include "stoplimit.h"
#include "limit.h"

queue *tsq;
queue *tbq;


void *stoplimitWorker(void *arg){
	order o1;
	char tr = 0;
	
	while(1){
		tr++;

		pthread_mutex_lock(tsq->mut);
		pthread_mutex_lock(price_mut);

		if((tsq->empty == 0) && (currentPriceX10 <= tsq->item[tsq->head].price1) ){
			pthread_mutex_unlock(price_mut);
			pthread_mutex_unlock(tsq->mut);

			qSafeDelete(tsq,&o1);
			o1.type = 'L';
			o1.price1 = o1.price2;
			qSafeSortAdd(lsq,o1);
			tr = 0;

		} else {
			pthread_mutex_unlock(price_mut);
			pthread_mutex_unlock(tsq->mut);
		}

		pthread_mutex_lock(tbq->mut);
		pthread_mutex_lock(price_mut);


		if((tbq->empty == 0) && (currentPriceX10 >= tbq->item[tbq->head].price1) ){
			pthread_mutex_unlock(price_mut);
			pthread_mutex_unlock(tbq->mut);
			qSafeDelete(tbq,&o1);
			o1.type = 'L';
			o1.price1 = o1.price2;
			qSafeSortAdd(lbq,o1);
			tr = 0;

		} else {
			pthread_mutex_unlock(price_mut);
			pthread_mutex_unlock(tbq->mut);
		}

		if(tr >= 2){
			signalWait(slimit);
		}
	}
	return arg;
}
