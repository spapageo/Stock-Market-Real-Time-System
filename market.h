#ifndef MARKET_H
#define MARKET_H

queue *msq;
queue *mbq;

void *marketWorker(void *arg);

void mmPairDelete();
void qlPairDelete(queue *q, queue *l);

#endif
