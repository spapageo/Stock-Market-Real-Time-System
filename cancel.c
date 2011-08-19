#include "marketSim.h"
#include "cancel.h"
#include "market.h"
#include "limit.h"
#include "stop.h"
#include "stoplimit.h"

queue *cq;


// ***********************************************
void *cancelWorker(void *arg){
	order ord;
	char type;
	queue *q = NULL;
	char result;
	while (1){
		qSafeDelete(cq,&ord);

		pthread_mutex_lock(saves.mut);
		type = saves.archive[ord.oldid];
		pthread_mutex_unlock(saves.mut);
		
		switch ( type ) {
			case 'M':
				q = msq;
				break;
			case 'N':
				q = mbq;
				break;
			case 'L':
				q = lsq;
				break;
			case 'K':
				q = lbq;
				break;
			case 'S':
				q = ssq;
				break;
			case 'P':
				q = sbq;
				break;
			case 'T':
				q = tsq;
				break;
			case 'W':
				q = tbq;
				break;
			case 'C':
				q = NULL;
				break;
			default:
				q = NULL;
				break;
		}

		if( q != NULL ){
			pthread_mutex_lock(q->mut);
			result = queueSearchDelete(q,ord.oldid);
			if (result == 0) pthread_cond_broadcast(q->notFull);
			pthread_mutex_unlock(q->mut);
		}
		
	}
	return arg;
}
