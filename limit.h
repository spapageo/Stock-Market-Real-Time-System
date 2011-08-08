#ifndef LIMIT_H
#define LIMIT_H

#include "marketSim.h"

extern llist *lsl;
extern llist *lbl;

void lSafeAdd(llist *l,order ord);
void lSafeDelete(llist *l,order *ord);
void llPairDelete(llist *sl, llist *bl);
void *limitWorker(void *arg);
int lGetHead(llist *l,order *o);

#endif
