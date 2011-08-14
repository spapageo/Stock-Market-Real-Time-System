#include "marketSim.h"
#include "stop.h"
#include "market.h"

queue *ssq;
queue *sbq;

void *stopWorker(void *arg){
	order o1;
	char done = 0;
	while(1){
		pthread_mutex_lock(ssq->mut);
		pthread_mutex_lock(price_mut);
		
		if(ssq->empty == 0){
			if( currentPriceX10 <= ssq->item[ssq->head].price1 ){
				pthread_mutex_unlock(price_mut);
				pthread_mutex_unlock(ssq->mut);
				qSafeDelete(ssq,&o1);
				o1.type = 'M';
				qSafeAdd(msq,o1);
				done = 1;
			}
		}
		
		if(done == 0){
			pthread_mutex_unlock(price_mut);
			pthread_mutex_unlock(ssq->mut);
		}else{
			done = 0;
		}

		pthread_mutex_lock(sbq->mut);
		pthread_mutex_lock(price_mut);

		if(sbq->empty == 0){
			if( currentPriceX10 >= sbq->item[sbq->head].price1 ){
				pthread_mutex_unlock(price_mut);
				pthread_mutex_unlock(sbq->mut);
				qSafeDelete(sbq,&o1);
				o1.type = 'M';
				qSafeAdd(mbq,o1);
				done = 1;
			}
		}

		if(done == 0){
			pthread_mutex_unlock(price_mut);
			pthread_mutex_unlock(sbq->mut);
		}else{
			done = 0;
		}

	}
	return arg;
}
