#ifndef LIMIT_H
#define LIMIT_H

llist *lsl;
llist *lbl;

void lSafeAdd(llist *l,order ord);
void lSafeDelete(llist *l,order *ord);
void llPairDelete(llist *l1, llist *l2);
order *lGetHead(llist *l);

#endif
