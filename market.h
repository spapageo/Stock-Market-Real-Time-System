#ifndef MARKET_H
#define MARKET_H

queue *msq;
queue *mbq;

void *marketWorker(void *arg);

void mQueueAdd(queue *q,order arg);
void mQueueDelete(queue *q,order *arg);
order *mQGetFirstOrder(queue *q);
void mPairDelete();

#endif
