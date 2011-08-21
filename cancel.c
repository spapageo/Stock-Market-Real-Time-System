/**
 * 
 * Copyright (c) 2011 Spyridwn Papageorgiou
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * *The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **/

#include "marketSim.h"
#include "cancel.h"
#include "market.h"
#include "limit.h"
#include "stop.h"
#include "stoplimit.h"

queue *cq;


/*
 * This is the worker thread the manages the cancel orders.
 * It first check the archive to find what type is assosiated with the supplied id.
 * According the that type select that queue and search for that id in order to delete it.
 * If it is not found there and the order is a 2-stage order like a stoplimit order
 * in searhc in the queue of the next stage of the order for that id.
 */
void *cancelWorker(void *arg){
	order ord;
	char type;
	queue *q[2];
	int i,iter;
	char result;
	while (1){
		qSafeDelete(cq,&ord);

		pthread_mutex_lock(saves.mut);
		type = saves.archive[ord.oldid];
		pthread_mutex_unlock(saves.mut);
		
		switch ( type ) {
			case 'M':
				q[0] = msq;
				iter = 1;
				break;
			case 'N':
				q[0] = mbq;
				iter = 1;
				break;
			case 'L':
				q[0] = lsq;
				iter = 1;
				break;
			case 'K':
				q[0] = lbq;
				iter = 1;
				break;
			case 'S':
				q[0] = ssq;
				q[1] = msq;
				iter = 2;
				break;
			case 'P':
				q[0] = sbq;
				q[1] = mbq;
				iter = 2;
				break;
			case 'T':
				q[0] = tsq;
				q[1] = lsq;
				iter = 2;
				break;
			case 'W':
				q[0] = tbq;
				q[1] = lbq;
				iter = 2;
				break;
			case 'C':
				q[0] = NULL;
				iter = 0;
				break;
			default:
				q[0] = NULL;
				iter = 0;
				break;
		}

		for(i = 0,result = -1; (i < iter) && (result != 0); i++){
			pthread_mutex_lock(q[i]->mut);
			result = queueSearchDelete(q[i],ord.oldid);
			if (result == 0) pthread_cond_broadcast(q[i]->notFull);
			pthread_mutex_unlock(q[i]->mut);
		}
		
	}
	return arg;
}
