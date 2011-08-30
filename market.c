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

#include <stdlib.h>
#include "marketSim.h"
#include "market.h"
#include "limit.h"

/*
 * This is the worker thread that manages the market orders. It perform the bulk of the transactions.
 */
void *marketWorker(void *arg) {

	char none,s;
	while (1) {
		none = 0;
		s = 0;
		/*
		 * Here we check if the requirements to perform a market buy - limit sell transaction are met.
		 * If so we perform the transaction.
		 */
		pthread_mutex_lock(mbq->mut);
		pthread_mutex_lock(lsq->mut);
		pthread_mutex_lock(price_mut);
		
		if ((mbq->empty == 0) && (lsq->empty == 0) && (lsq->item[lsq->head].price1 <= currentPriceX10)) {
			mlPairDelete( mbq, lsq , log_file);
			none=1;
			s=1;
		}
		
		pthread_mutex_unlock(price_mut);
		pthread_mutex_unlock(lsq->mut);
		pthread_mutex_unlock(mbq->mut);
		
		/*
		 * If a transaction has occured, we wake up the stop and stoplimit threads
		 * to recheck the value of the current price
		 */
		if(s == 1)	signalSend(slimit);
		s = 0;
		
		/*
		 * Here we check if the requirements to perform a market sell - limit buy transaction are met.
		 * If so we perform the transaction.
		 */
		pthread_mutex_lock(msq->mut);
		pthread_mutex_lock(lbq->mut);
		pthread_mutex_lock(price_mut);
		
		
		if((msq->empty == 0) && (lbq->empty == 0) && (lbq->item[lbq->head].price1 >= currentPriceX10)){
			mlPairDelete( msq, lbq , log_file);
			none=1;
			s=1;
		}
		
		pthread_mutex_unlock(price_mut);
		pthread_mutex_unlock(lbq->mut);
		pthread_mutex_unlock(msq->mut);

		/*
		 * If a transaction has occured, we wake up the stop and stoplimit threads
		 * to recheck the value of the current price
		 */
		if(s == 1)	signalSend(slimit);
		
		
		/*
		 * Here we check if none of the above trasactions have occured. If not
		 * we perform a market-market transaction
		 */
		pthread_mutex_lock(msq->mut);
		pthread_mutex_lock(mbq->mut);
		
		if ( (msq->empty == 0) && (mbq->empty == 0) && (none == 0) ) {
			mmPairDelete(msq, mbq, log_file);
			none = 2;
		}
		
		pthread_mutex_unlock(mbq->mut);
		pthread_mutex_unlock(msq->mut);

		/*
		 * If no no market - limit transaction has happened we signal the limit thread to try
		 * and perform a limit - limit transaction
		 */
		if(none == 2)	signalSend(lim);
		
		fflush(log_file);
	}
	return arg;
}

/* Performs the transaction between the first order from both market queues */
void mmPairDelete( queue * sq, queue * bq, FILE * lf ) {
	
	/*
	 * Perform the transaction, print it in the logfile
	 */
	int vol1 = sq->item[sq->head].vol;
	int vol2 = bq->item[bq->head].vol;
	int id1 = sq->item[sq->head].id;
	int id2 = bq->item[bq->head].id;
	order ord;

	/* Checks which order has the larger volume */
	if (vol1 < vol2) {
		
		vol2 = vol2 - vol1;
		queueDel(sq,&ord);
		qGetFirst(bq)->vol = vol2;
		pthread_cond_broadcast(sq->notFull);
		
	} else if (vol1 > vol2) {
		
		vol1 = vol1 - vol2;
		queueDel(bq,&ord);
		qGetFirst(sq)->vol = vol1;
		pthread_cond_broadcast(bq->notFull);
		
	} else {
		
		queueDel(bq,&ord);
		queueDel(sq,&ord);
		pthread_cond_broadcast(sq->notFull);
		pthread_cond_broadcast(bq->notFull);
		
	}

	/* Logs the transaction to the file */
	fprintf(lf,"%08ld	%08ld	%5.1f	%05d	%08d	%08d\n", ord.timestamp, getTimestamp(), (float)currentPriceX10/10, vol1,id1,id2);
}

/*
 *  Performs the trasaction between a limit and a market order and prints it to the log file.
 */
void mlPairDelete( queue*  m, queue*  l, FILE* lf ){

	int vol1 = m->item[m->head].vol, vol2 = l->item[l->head].vol;
	int pvol = 0;
	int id1 = m->item[m->head].id, id2 = l->item[l->head].id;
	order ord;
	
	/* Checks which order has the larger volume */
	if (vol1 < vol2) {
		vol2 = vol2 - vol1;
		pvol = vol1;
		queueDel(m,&ord);
		l->item[l->head].vol = vol2;
		pthread_cond_broadcast(m->notFull);
		currentPriceX10 = l->item[l->head].price1;
	} else if (vol1 > vol2) {
		vol1 = vol1 - vol2;
		pvol = vol2;
		queueDel(l,&ord);
		qGetFirst(m)->vol = vol1;
		pthread_cond_broadcast(l->notFull);
		currentPriceX10 = ord.price1;
	} else {
		pvol = vol1;
		queueDel(m,&ord);
		queueDel(l,&ord);
		pthread_cond_broadcast(m->notFull);
		pthread_cond_broadcast(l->notFull);
		currentPriceX10 = ord.price1;
	}

	/* Logs the transaction to the file */
	fprintf(lf,"%08ld	%08ld	%5.1f	%05d	%08d	%08d\n", ord.timestamp, getTimestamp(),(float) currentPriceX10/10, pvol, id1, id2);

}
