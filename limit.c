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
#include "limit.h"

// *********************************************************
queue *lsq;
queue *lbq;

// *********************************************************
void *limitWorker(void *arg){
	char none;
	
	while (1) {
		none = 0;

		signalWait(lim);

		
		pthread_mutex_lock(lsq->mut);
		pthread_mutex_lock(lbq->mut);
		pthread_mutex_lock(price_mut);
		
		
		if((lsq->empty == 0) && (lbq->empty == 0) && (none == 0)) {
			if ((currentPriceX10 >= lsq->item[lsq->head].price1) && (currentPriceX10 <= lbq->item[lbq->head].price1)) {
				llPairDelete(lsq,lbq);
				none = 1;
			}
		}
		
		pthread_mutex_unlock(price_mut);
		pthread_mutex_unlock(lbq->mut);
		pthread_mutex_unlock(lsq->mut);

		if (none == 1)	signalSend(slimit);

		fflush(log_file);
	}
	return arg;
}

void llPairDelete(queue *sl, queue *bl){

	int vol1 = sl->item[sl->head].vol, vol2 = bl->item[bl->head].vol;
	int pvol = 0;
	int id1 = sl->item[sl->head].id, id2 = bl->item[bl->head].id;
	order ord;
	if (vol1 < vol2) {
		currentPriceX10 = ( sl->item[sl->head].price1 + bl->item[bl->head].price1)/2;
		vol2 = vol2 - vol1;
		pvol = vol1;
		queueDel(sl,&ord);
		bl->item[bl->head].vol = vol2;
		
		pthread_cond_broadcast(sl->notFull);
	} else if (vol1 > vol2) {
		currentPriceX10 = ( sl->item[sl->head].price1 + bl->item[bl->head].price1)/2;
		vol1 = vol1 - vol2;
		pvol = vol2;
		queueDel(bl,&ord);
		sl->item[sl->head].vol = vol1;
		pthread_cond_broadcast(bl->notFull);
	} else {
		currentPriceX10 = ( sl->item[sl->head].price1 + bl->item[bl->head].price1)/2;
		queueDel(sl,&ord);
		queueDel(bl,&ord);
		pvol = vol1;
		pthread_cond_broadcast(sl->notFull);
		pthread_cond_broadcast(bl->notFull);
	}
	
	fprintf(log_file,"%08ld	%08ld	%5.1f	%05d	%08d	%08d\n", ord.timestamp, getTimestamp(), (float)currentPriceX10/10, pvol, id1, id2);

}
