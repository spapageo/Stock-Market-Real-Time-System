#include "marketSim.h"
#include "stoplimit.h"
#include "limit.h"

queue *tsq;
queue *tbq;


void *stoplimitWorker(void *arg){
	order o1;
	while(1){
		
		
		if(tsq->empty == 0){
			if( currentPriceX10 >= tsq->item[tsq->head].price1 ){
				qSafeDelete(tsq,&o1);
				o1.type = 'L';
				o1.price1 = o1.price2;
				qSafeSortAdd(lsq,o1);
			}
		}
		
		if(tbq->empty == 0){
			if( currentPriceX10 >= tbq->item[tbq->head].price1 ){
				qSafeDelete(tbq,&o1);
				o1.type = 'L';
				o1.price1 = o1.price2;
				qSafeSortAdd(lbq,o1);
			}
		}
		
	}
	return arg;
}