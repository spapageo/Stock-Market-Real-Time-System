#include <stdio.h>
#include <stdlib.h>
#include "marketSim.h"
#include "market.h"

void mQueueAdd(queue *q,order arg) {
// 	printf("Will try mQueueAdd\n");
// 	fflush(stdout);
    pthread_mutex_lock (q->mut);
    while (q->full) {
        printf ("*** Incoming %c Market Order Queue is FULL.\n",arg.action);
        fflush(stdout);
        pthread_cond_wait (q->notFull, q->mut);
    }
    queueAdd (q, arg);
    pthread_mutex_unlock (q->mut);
    pthread_cond_signal (q->notEmpty);
}

void mQueueDelete(queue *q,order *arg) {
// 	printf("Will try mQueueDelete\n");
// 	fflush(stdout);
    pthread_mutex_lock (q->mut);
    while (q->empty) {
        printf ("*** Incoming %c Market Queue is EMPTY.\n",arg->action);
        fflush(stdout);
        pthread_cond_wait (q->notEmpty, q->mut);
    }
    queueDel (q, arg);

    pthread_mutex_unlock (q->mut);
    pthread_cond_signal (q->notFull);
}



void *marketWorker(void *arg) {
    queue *q = (queue *)arg;
    while (1) {
        //Check if a pair of market orders is available
        if (!msq->empty && !mbq->empty) {
            mPairDelete();
        }

        inputConsumer(q);
    }
    return NULL;
}

void mPairDelete() {
    // Lock both queues
    pthread_mutex_lock(msq->mut);
    pthread_mutex_lock(mbq->mut);

    //Check if a pair of market orders is available
    if ((!msq->empty) && (!mbq->empty)) {

        int vol1 = msq->item[msq->head].vol;
        int vol2 = mbq->item[mbq->head].vol;
        order ord;
        if (vol1 < vol2) {
            vol2 = vol2 - vol1;
            queueDel(msq,&ord);
            mQueuegetOrder(mbq)->vol = vol2;
            pthread_cond_signal(msq->notFull);
        } else if (vol1 > vol2) {
            vol1 = vol1 - vol2;
            queueDel(mbq,&ord);
            mQueuegetOrder(msq)->vol = vol1;
			pthread_cond_signal(mbq->notFull);
        } else {
            queueDel(mbq,&ord);
            queueDel(msq,&ord);
            pthread_cond_signal(msq->notFull);
            pthread_cond_signal(mbq->notFull);
        }
    }

    pthread_mutex_unlock(msq->mut);
    pthread_mutex_unlock(mbq->mut);

}

order *mQueuegetOrder(queue *q) {
    return &(q->item[q->head]);
}
