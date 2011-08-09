#include "marketSim.h"
#include "stoplimit.h"
#include "limit.h"

llist *tsl;
llist *tbl;


void *stoplimitWorker(void *arg){
	int switch1, switch2;
	order o1,o2;
	o1.price1 = 1000;
	o2.price1 = 1000;
	while(1){
		switch1 = lGetHead(tsl,&o1);
		switch2 = lGetHead(tbl,&o2);
		
		if(switch1 == 1){
			if( currentPriceX10 <= o1.price1 ){
				lSafeDelete(tsl,&o1);
				o1.type = 'L';
				o1.price1 = o1.price2;
				lSafeAdd(lsl,o1);
			}
		}
		
		if(switch2 == 1){
			if( currentPriceX10 >= o2.price1 ){
				lSafeDelete(tbl,&o2);
				o2.type = 'L';
				o2.price1 = o2.price2;
				lSafeAdd(lbl,o2);
			}
		}
	}
	return NULL;
}