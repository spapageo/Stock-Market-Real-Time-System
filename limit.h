#ifndef LIMIT_H
#define LIMIT_H

#include "marketSim.h"

extern queue *lsq;
extern queue *lbq;

void llPairDelete(queue *sl, queue *bl);
void *limitWorker(void *arg);

#endif
