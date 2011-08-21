/**
 * 
 * Copyright (c) 2011 Spyridwn Papageorgiou
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * *The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **/

#include "marketSim.h"
#include "stoplimit.h"
#include "limit.h"

queue *tsq;
queue *tbq;

/*
 * This is the worker thread that manages the stoplimit orders.
 * It checks if the current price if suitable so it can convert
 * the order to a market order.
 * If after 2 tries it hasn't converted any orders it waits for a signal
 * that the current price has changed in order to try again.
 */
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

		/*
		 * If no convertion has happened for 2 iterations we
		 * wait for a signal that the current stock price has changed.
		 */
		if(tr >= 2){
			signalWait(slimit);
		}
	}
	return arg;
}

