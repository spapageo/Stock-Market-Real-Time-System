#include "marketSim.h"
#include "stoplimit.h"
#include "limit.h"

queue *tsq;
queue *tbq;


void *stoplimitWorker(void *arg){
	order o1;
	char done = 0;
	while(1){
		pthread_mutex_lock(tsq->mut);
		pthread_mutex_lock(price_mut);
		
		
		if(tsq->empty == 0){
			if( currentPriceX10 >= tsq->item[tsq->head].price1 ){
				pthread_mutex_unlock(price_mut);
				pthread_mutex_unlock(tsq->mut);
				qSafeDelete(tsq,&o1);
				o1.type = 'L';
				o1.price1 = o1.price2;
				qSafeSortAdd(lsq,o1);
				done = 1;
			}
		}

		if(done == 0){
			pthread_mutex_unlock(price_mut);
			pthread_mutex_unlock(tsq->mut);
		}else{
			done = 0;
		}

		pthread_mutex_lock(tbq->mut);
		pthread_mutex_lock(price_mut);
		
		if(tbq->empty == 0){
			if( currentPriceX10 >= tbq->item[tbq->head].price1 ){
				pthread_mutex_unlock(price_mut);
				pthread_mutex_unlock(tbq->mut);
				qSafeDelete(tbq,&o1);
				o1.type = 'L';
				o1.price1 = o1.price2;
				qSafeSortAdd(lbq,o1);
				done = 1;
			}
		}

		if(done == 0){
			pthread_mutex_unlock(price_mut);
			pthread_mutex_unlock(tbq->mut);
		}else{
			done = 0;
		}
		
	}
	return arg;
}
