#include "marketSim.h"
#include "stop.h"
#include "limit.h"
#include "market.h"

queue *ssq;
queue *sbq;

// void *stopWorker(void *arg){
// 	int switch1, switch2;
// 	order o1,o2;
// 	o1.price1 = 1000;
// 	o2.price1 = 1000;
// 	while(1){
// 		switch1 = lGetHead(ssl,&o1);
// 		switch2 = lGetHead(sbl,&o2);
// 
// 		if(switch1 == 1){
// 			if( currentPriceX10 <= o1.price1 ){
// 				lSafeDelete(ssl,&o1);
// 				o1.type = 'M';
// 				qSafeAdd(msq,o1);
// 			}
// 		}
// 
// 		if(switch2 == 1){
// 			if( currentPriceX10 >= o2.price1 ){
// 				lSafeDelete(sbl,&o2);
// 				o2.type = 'M';
// 				qSafeAdd(mbq,o2);
// 			}
// 		}
// 	}
// 	return NULL;
// }