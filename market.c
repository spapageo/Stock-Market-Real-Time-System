#include <stdlib.h>
#include "marketSim.h"
#include "market.h"
#include "limit.h"

void qSafeAdd(queue *q,order arg) {
	pthread_mutex_lock (q->mut);
	while (q->full) {
		printf ("*** Incoming %c Market Order Queue is FULL.\n",arg.action);
		fflush(stdout);
		pthread_cond_wait (q->notFull, q->mut);
	}
	queueAdd (q, arg);
	pthread_cond_signal (q->notEmpty);
	pthread_mutex_unlock (q->mut);
}

void qSafeDelete(queue *q,order *arg) {

	pthread_mutex_lock (q->mut);
	while (q->empty) {
		pthread_cond_wait (q->notEmpty, q->mut);
	}
	queueDel (q, arg);

	pthread_cond_signal (q->notFull);
	pthread_mutex_unlock (q->mut);
	
}



void *marketWorker(void *arg) {
	int none=0;
	while (1) {
		none = 0;
		if (!msq->empty && !lbl->empty){
			if(lbl->HEAD->ord.price1 > currentPriceX10){
				qlPairDelete( msq, lbl );
				none=1;
			}
		}
		
		if (!mbq->empty && !lsl->empty){
			if(lsl->HEAD->ord.price1 < currentPriceX10){
				qlPairDelete( mbq, lsl );
				none=1;
			}
		}
		
		if (!(msq->empty) && !(mbq->empty) && none == 0){
			qqPairDelete();
		}
	}
	return NULL;
}

void qqPairDelete() {
	// Lock both queues
	pthread_mutex_lock(msq->mut);
	pthread_mutex_lock(mbq->mut);
	pthread_mutex_lock(price_mut);
	
	//Check if a pair of market orders is available
	if ((!msq->empty) && (!mbq->empty)) {

		int vol1 = msq->item[msq->head].vol;
		int vol2 = mbq->item[mbq->head].vol;
		long int id1 = msq->item[msq->head].id;
		long int id2 = mbq->item[mbq->head].id;
		order ord;
		if (vol1 < vol2) {
			vol2 = vol2 - vol1;
			queueDel(msq,&ord);

			fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol1,id1,id2);
			fflush(log_file);
			
			qGetFirst(mbq)->vol = vol2;
			pthread_cond_signal(msq->notFull);
		} else if (vol1 > vol2) {
			vol1 = vol1 - vol2;
			queueDel(mbq,&ord);

			fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol2,id1,id2);
			fflush(log_file);
			
			qGetFirst(msq)->vol = vol1;
			pthread_cond_signal(mbq->notFull);
		} else {
			queueDel(mbq,&ord);
			queueDel(msq,&ord);

			fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol1,id1,id2);
			fflush(log_file);
			
			pthread_cond_signal(msq->notFull);
			pthread_cond_signal(mbq->notFull);
		}
	}
	
	pthread_mutex_unlock(price_mut);
	pthread_mutex_unlock(msq->mut);
	pthread_mutex_unlock(mbq->mut);

}

order *qGetFirst(queue *q) {
	return &(q->item[q->head]);
}

void qlPairDelete( queue *q, llist *l ){
	//Lock both the list and the queue
	pthread_mutex_lock(l->mut);
	pthread_mutex_lock(q->mut);
	pthread_mutex_lock(price_mut);
	
	if( !q->empty && !l->empty ){
		if (l->HEAD->ord.action == 'S' && l->HEAD->ord.price1 < currentPriceX10){
			int vol1 = q->item[q->head].vol;
			int vol2 = l->HEAD->ord.vol;
			long int id1 = q->item[q->head].id;
			long int id2 = l->HEAD->ord.id;
			order ord;
			
			if (vol1 < vol2) {
				vol2 = vol2 - vol1;
				queueDel(q,&ord);
				lGetHead(l)->vol = vol2;
				pthread_cond_signal(q->notFull);
				
				currentPriceX10 = lGetHead(l)->price1;
				fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol1, id1, id2);
				fflush(log_file);
				
			} else if (vol1 > vol2) {
				vol1 = vol1 - vol2;
				llistDel(l,&ord);
				qGetFirst(q)->vol = vol1;
				pthread_cond_signal(l->notFull);

				currentPriceX10 = ord.price1;
				fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol2, id1, id2);
				fflush(log_file);
			} else {
				queueDel(q,&ord);
				llistDel(l,&ord);
				pthread_cond_signal(q->notFull);
				pthread_cond_signal(l->notFull);

				currentPriceX10 = ord.price1;
				fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol2, id1, id2);
				fflush(log_file);
			}
		} else if(l->HEAD->ord.action == 'B' && l->HEAD->ord.price1 > currentPriceX10){
			int vol1 = q->item[q->head].vol;
			int vol2 = l->HEAD->ord.vol;
			long int id1 = q->item[q->head].id;
			long int id2 = l->HEAD->ord.id;
			order ord;
			if (vol1 < vol2) {
				vol2 = vol2 - vol1;
				queueDel(q,&ord);
				lGetHead(l)->vol = vol2;
				pthread_cond_signal(q->notFull);

				currentPriceX10 = lGetHead(l)->price1;
				fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol1, id1, id2);
				fflush(log_file);
			} else if (vol1 > vol2) {
				vol1 = vol1 - vol2;
				llistDel(l,&ord);
				qGetFirst(q)->vol = vol1;
				pthread_cond_signal(l->notFull);

				currentPriceX10 = ord.price1;
				fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol2, id1, id2);
				fflush(log_file);
			} else {
				queueDel(q,&ord);
				llistDel(l,&ord);
				pthread_cond_signal(q->notFull);
				pthread_cond_signal(l->notFull);

				currentPriceX10 = ord.price1;
				fprintf(log_file,"%08ld	%5.1f	%05d	%08ld	%08ld\n",getTimestamp(),(float) currentPriceX10/10, vol2, id1, id2);
				fflush(log_file);
			}
		}
	}
	
	//Unlock both the list and the queue
	pthread_mutex_unlock(price_mut);
	pthread_mutex_unlock(l->mut);
	pthread_mutex_unlock(q->mut);
}
