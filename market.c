#include <stdlib.h>
#include "marketSim.h"
#include "market.h"
#include "limit.h"

<<<<<<< .merge_file_XM2elu
void qSafeAdd(queue *q,order arg) {
	pthread_mutex_lock (q->mut);
	while (q->full) {
		pthread_cond_wait (q->notFull, q->mut);
	}
	queueAdd (q, arg);
	pthread_cond_broadcast(q->notEmpty);
	pthread_mutex_unlock(q->mut);
}

void qSafeDelete(queue *q,order *arg) {

	pthread_mutex_lock(q->mut);
	while (q->empty){
		pthread_cond_wait(q->notEmpty, q->mut);
	}
	queueDel(q, arg);

	pthread_cond_broadcast(q->notFull);
	pthread_mutex_unlock(q->mut);
	
}


void *marketWorker(void *arg) {

	char none;
	while (1) {
		none = 0;
		// **********************************************************

		
		pthread_mutex_lock(mbq->mut);
		pthread_mutex_lock(lsq->mut);
		pthread_mutex_lock(price_mut);
		
		if ((mbq->empty == 0) && (lsq->empty == 0) && (lsq->item[lsq->head].price1 <= currentPriceX10)) {
			mlPairDelete( mbq, lsq );
			none=1;
		}
		
		pthread_mutex_unlock(price_mut);
		pthread_mutex_unlock(lsq->mut);
		pthread_mutex_unlock(mbq->mut);
		
		// **********************************************************
		pthread_mutex_lock(msq->mut);
		pthread_mutex_lock(lbq->mut);
		pthread_mutex_lock(price_mut);
		
		
		if((msq->empty == 0) && (lbq->empty == 0) && (lbq->item[lbq->head].price1 >= currentPriceX10)){
			mlPairDelete( msq, lbq );
			none=1;
		}
		
		pthread_mutex_unlock(price_mut);
		pthread_mutex_unlock(lbq->mut);
		pthread_mutex_unlock(msq->mut);
		
		// **********************************************************

		
		pthread_mutex_lock(msq->mut);
		pthread_mutex_lock(mbq->mut);
		
		if ( (msq->empty == 0) && (mbq->empty == 0) && (none == 0) ) {
			mmPairDelete();
			none = 2;
		}
		
		pthread_mutex_unlock(mbq->mut);
		pthread_mutex_unlock(msq->mut);

		if(none != 0)	signalSend(slimit);
		if(none == 2)	signalSend(lim);
		
		fflush(log_file);
	}
	return arg;
}

/* Check if both queues have at least one order available and deletes the first order from both market queues */
void mmPairDelete() {
	
	/*
	 * Perform the transaction, print it in the logfile
	 */
	int vol1 = msq->item[msq->head].vol;
	int vol2 = mbq->item[mbq->head].vol;
	int id1 = msq->item[msq->head].id;
	int id2 = mbq->item[mbq->head].id;
	order ord;
	
	if (vol1 < vol2) {
		
		vol2 = vol2 - vol1;
		queueDel(msq,&ord);
		qGetFirst(mbq)->vol = vol2;
		pthread_cond_broadcast(msq->notFull);
		
	} else if (vol1 > vol2) {
		
		vol1 = vol1 - vol2;
		queueDel(mbq,&ord);
		qGetFirst(msq)->vol = vol1;
		pthread_cond_broadcast(mbq->notFull);
		
	} else {
		
		queueDel(mbq,&ord);
		queueDel(msq,&ord);
		pthread_cond_broadcast(msq->notFull);
		pthread_cond_broadcast(mbq->notFull);
		
	}
	fprintf(log_file,"%08ld	%08ld	%5.1f	%05d	%08d	%08d\n", ord.timestamp, getTimestamp(), (float)currentPriceX10/10, vol1,id1,id2);
}

/*
 *  Performs the trasaction between a limit and a market order and prints it to the log file.
 */
void mlPairDelete( queue *m, queue *l ){

		int vol1 = m->item[m->head].vol, vol2 = l->item[l->head].vol;
		int pvol = 0;
		int id1 = m->item[m->head].id, id2 = l->item[l->head].id;
		order ord;
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
		fprintf(log_file,"%08ld	%08ld	%5.1f	%05d	%08d	%08d\n", ord.timestamp, getTimestamp(),(float) currentPriceX10/10, pvol, id1, id2);

}
