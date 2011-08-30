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
#include "stop.h"
#include "market.h"

queue *ssq;
queue *sbq;

/*
 * This is the worker thread that manages the stop orders.
 * It checks if the current price if suitable so it can convert
 * the order to a limit order.
 * If after 2 tries it hasn't converted any orders it waits for a signal
 * that the current price has changed in order to try again.
 */
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
			o1.timestamp = getTimestamp();
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
			o1.timestamp = getTimestamp();
			qSafeAdd(mbq,o1);

			tr = 0;
		} else {
			pthread_mutex_unlock(sbq->mut);
			pthread_mutex_unlock(price_mut);
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

