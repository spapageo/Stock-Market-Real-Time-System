#include "marketSim.h"
#include "stop.h"
#include "market.h"

queue *ssq;
queue *sbq;

void *stopWorker(void *arg){
	order o1;
	while(1){


		if(ssq->empty == 0){
			if( currentPriceX10 <= ssq->item[ssq->head].price1 ){
				qSafeDelete(ssq,&o1);
				o1.type = 'M';
				qSafeAdd(msq,o1);
			}
		}

		if(sbq->empty == 0){
			if( currentPriceX10 >= sbq->item[sbq->head].price1 ){
				qSafeDelete(sbq,&o1);
				o1.type = 'M';
				qSafeAdd(mbq,o1);
			}
		}

	}
	return arg;
}