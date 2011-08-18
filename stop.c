#include "marketSim.h"
#include "stop.h"
#include "market.h"

queue *ssq;
queue *sbq;

void *stopWorker(void *arg){
	order o1;
	char tr = 0;
	while(1){
		tr++;
		
		pthread_mutex_lock(price_mut);
		pthread_mutex_lock(ssq->mut);
		
		if(ssq->empty == 0 && currentPriceX10 <= ssq->item[ssq->head].price1 ){
			pthread_mutex_unlock(ssq->mut);
			pthread_mutex_unlock(price_mut);

			qSafeDelete(ssq,&o1);
			o1.type = 'M';
			qSafeAdd(msq,o1);
			tr = 0;
		} else{
			pthread_mutex_unlock(ssq->mut);
			pthread_mutex_unlock(price_mut);
		}

		pthread_mutex_lock(price_mut);
		pthread_mutex_lock(sbq->mut);

		if(sbq->empty == 0 && currentPriceX10 >= sbq->item[sbq->head].price1 ){
			pthread_mutex_unlock(sbq->mut);
			pthread_mutex_unlock(price_mut);

			qSafeDelete(sbq,&o1);
			o1.type = 'M';
			qSafeAdd(mbq,o1);

			tr = 0;
		} else {
			pthread_mutex_unlock(sbq->mut);
			pthread_mutex_unlock(price_mut);
		}


		
		if(tr >= 1){
			signalWait(slimit);
		}
	}
	return arg;
}

