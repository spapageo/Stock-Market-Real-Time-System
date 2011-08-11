#ifndef MARKET_H
#define MARKET_H

queue *msq;
queue *mbq;

void *marketWorker(void *arg);

void mmPairDelete();
void mlPairDelete(queue *q, queue *l);

#endif
