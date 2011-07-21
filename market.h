#ifndef MARKET_H
#define MARKET_H

queue *msq;
queue *mbq;

void *marketWorker(void *arg);

void qSafeAdd(queue *q,order arg);
void qSafeDelete(queue *q,order *arg);
order *qGetFirst(queue *q);
void qqPairDelete();
void qlPairDelete(queue *q, llist *l);

#endif
