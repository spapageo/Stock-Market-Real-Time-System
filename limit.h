#ifndef STOP_H
#define STOP_H

llist *lsl;
llist *lbl;

void lSafeAdd(llist *l,order ord);
void lSafeDelete(llist *l,order *ord);
order *lGetHead(llist *l);

#endif

