#ifndef LIMIT_H
#define LIMIT_H

#include "marketSim.h"

extern queue *lsl;
extern queue *lbl;

void qSafeSortAdd(queue *q,order ord);
void lSafeDelete(llist *l,order *ord);
void llPairDelete(llist *sl, llist *bl);
void *limitWorker(void *arg);
int lGetHead(llist *l,order *o);

#endif
