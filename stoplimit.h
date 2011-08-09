#ifndef STOPLIMIT_H
#define STOPLIMIT_H

#include "marketSim.h"

extern queue *tsq;
extern queue *tbq;

void *stoplimitWorker(void *arg);
#endif
