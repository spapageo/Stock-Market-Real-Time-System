#ifndef LIMIT_H
#define LIMIT_H

llist *lsl;
llist *lbl;

void lSafeAdd(llist *l,order ord);
void lSafeDelete(llist *l,order *ord);
void llPairDelete(llist *sl, llist *bl);
void *limitWorker(void *arg);
order *lGetHead(llist *l);

#endif
