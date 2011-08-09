#ifndef LIMIT_H
#define LIMIT_H

#include "marketSim.h"

extern queue *lsl;
extern queue *lbl;

void llPairDelete(queue *sl, queue *bl);
void *limitWorker(void *arg);

#endif
