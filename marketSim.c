#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include "marketSim.h"
#include "market.h"
#include "limit.h"
#include "stop.h"
#include "stoplimit.h"

// *****************************************************************
struct timeval startwtime, endwtime;

volatile int currentPriceX10;

rec saves;

pthread_mutex_t *price_mut;

FILE *log_file;

// ****************************************************************
int main() {

	// reset number generator seed
	srand(time(NULL) + getpid());
	//srand(0); // to get the same sequence
	
	//initialize the price, it's mutex, and it's condition variable
	currentPriceX10 = 1000;
	price_mut = malloc (sizeof(pthread_mutex_t));
	pthread_mutex_init(price_mut,NULL);

	//open the log file foe writing
	log_file = fopen("logfile.txt","w+");

	saves.archive = calloc(INT_MAX,sizeof(char));
	if(saves.archive == NULL) exit(0);
	
	// start the time for timestamps
	gettimeofday (&startwtime, NULL);

	//Create all the thread variables
	pthread_t prod, cons, cons2, market, lim_thread, stop_thread,slim_thread;
	
	// Initialize the incoming order queue
	queue *q = queueInit(0);
	
	// Initialize the buy and sell market queues.
	msq = queueInit(0);
	mbq = queueInit(0);

	// Initialize the buy and sell limit order lists
	lsq = queueInit(ASC);
	lbq = queueInit(DESC);
	
	// Initialize the buy and sell stop order lists
	ssq = queueInit(ASC);
	sbq = queueInit(DESC);

	// Initialize the buy and sell stop limit order lists
	tsq = queueInit(ASC);
	tbq = queueInit(DESC);

	printf("\n\n");
	
	// Create and launch all the appropriate threads
	
	pthread_create(&cons, NULL, Cons, q);
	pthread_create(&cons2, NULL, Cons, q);
	pthread_create(&lim_thread, NULL, limitWorker, NULL);
	pthread_create(&market, NULL, marketWorker, NULL);
	pthread_create(&stop_thread, NULL, stopWorker, NULL);
	pthread_create(&slim_thread, NULL, stoplimitWorker, NULL);
	pthread_create(&prod, NULL, Prod, q);
	
	// I actually do not expect them to ever terminate
	pthread_join(prod, NULL);


	pthread_exit(NULL);
}

// ****************************************************************
void *Prod (void *arg) {
	queue *q = (queue *) arg;
	while (1) {
		pthread_mutex_lock (q->mut);
		
		fputs("\033[A\033[2K",stdout);
		rewind(stdout);
		ftruncate(1,0);
		printf("**** %05d **** %05d **** %05d **** %05d **** %05d **** %05d **** %05d **** %05d **** %05d ****\n",q->size,msq->size,mbq->size,lsq->size,lbq->size,ssq->size,sbq->size,tsq->size,tbq->size);
		fflush(stdout);
		
		while (q->full) {
			pthread_cond_wait (q->notFull, q->mut);
		}
		queueAdd (q, makeOrder());
		pthread_cond_broadcast (q->notEmpty);
		pthread_mutex_unlock (q->mut);
	}
	return NULL;
}

// ****************************************************************
void *Cons (void *arg) {
	queue *q = (queue *) arg;
	order ord;

	while (1) {
		pthread_mutex_lock (q->mut);
		while (q->empty) {
			pthread_cond_wait (q->notEmpty, q->mut);
		}
		queueDel (q, &ord);

		pthread_cond_broadcast(q->notFull);
		pthread_mutex_unlock (q->mut);
		
		// YOUR CODE IS CALLED FROM HERE
		// Process that order!
		if (ord.type == 'M') {
			if(ord.action == 'S'){
				qSafeAdd(msq,ord);
				saves.archive[ord.id] = 'M';
			} else {
				qSafeAdd(mbq,ord);
				saves.archive[ord.id] = 'N';
			}
		} else if (ord.type == 'L') {
			if(ord.action == 'S'){
				qSafeSortAdd(lsq,ord);
				saves.archive[ord.id] = 'L';
			}else{
				qSafeSortAdd(lbq,ord);
				saves.archive[ord.id] = 'K';
			}
		} else if (ord.type == 'S') {
			if(ord.action == 'S'){
				qSafeSortAdd(ssq,ord);
				saves.archive[ord.id] = 'S';
			}else{
				qSafeSortAdd(sbq,ord);
				saves.archive[ord.id] = 'P';
			}
		} else if (ord.type == 'T') {
			if(ord.action == 'S'){
				qSafeSortAdd(tsq,ord);
				saves.archive[ord.id] = 'T';
			}else{
				qSafeSortAdd(tbq,ord);
				saves.archive[ord.id] = 'W';
			}
		} else {
			//Here we hadle the cancel commands
		}

	}
	return NULL;
}

// ****************************************************************
order makeOrder() {

	static int count = 0;
	int magnitude = 10;
	int waitmsec;

	order ord;

	// wait for a random amount of time in useconds
	waitmsec = ((double)rand()/(double)RAND_MAX * magnitude);
	usleep(waitmsec*1000);

	ord.id = count++;
	ord.oldid = 0;
	ord.timestamp = getTimestamp();

	// Buy or Sell
	ord.action = ((double)rand()/(double)RAND_MAX <= 0.5) ? 'B' : 'S';

	// Order type
	double u2 = ((double)rand()/(double)RAND_MAX);
	if (u2 < 0.2) {
		ord.type = 'M';				 // Market order
		ord.vol = (1 + rand()%50)*100;
		ord.price1 = 0;
		ord.price2 = 0;

	} else if (0.2 <= u2 && u2 < 0.4) {
		ord.type = 'L';				 // Limit order
		ord.vol = (1 + rand()%50)*100;
		ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));
		ord.price2 = 0;

	} else if (0.4 <= u2 && u2 < 0.6) {
		ord.type = 'S';				 // Stop order
		ord.vol = (1 + rand()%50)*100;
		ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));
		ord.price2 = 0;

	} else if (0.6 <= u2 && u2 < 0.8) {
		ord.type = 'T';				 // Stop Limit order
		ord.vol = (1 + rand()%50)*100;
		ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));
		ord.price2 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));

	} else {
		ord.type = 'C';				 // Cancel order
		ord.oldid = ((double)rand()/(double)RAND_MAX)*count;
		ord.price1 = 0;
		ord.price2 = 0;
	}

	return (ord);
}

// ****************************************************************
long getTimestamp() {

	gettimeofday(&endwtime, NULL);

	return((double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
					+ endwtime.tv_sec - startwtime.tv_sec)*1000);
}

// ****************************************************************
void dispOrder(order ord) {

	printf("%08d ", ord.id);
	printf("%08ld ", ord.timestamp);
	switch ( ord.type ) {
	case 'M':
		printf("%c ", ord.action);
		printf("Market (%4d)		", ord.vol);
		break;
	case 'L':
		printf("%c ", ord.action);
		printf("Limit  (%4d,%5.1f) ", ord.vol, (float) ord.price1/10.0);
		break;
	case 'S':
		printf("%c ", ord.action);
		printf("Stop   (%4d,%5.1f) ", ord.vol, (float) ord.price1/10.0);
		break;
	case 'T':
		printf("%c ", ord.action);
		printf("StopLim(%4d,%5.1f,%5.1f) ",
			   ord.vol, (float) ord.price1/10.0, (float) ord.price2/10.0);
		break;
	case 'C':
		printf("* Cancel  %6d		", ord.oldid);
		break;
	default :
		break;
	}
	printf("\n");
}

// ****************************************************************
queue *queueInit (int shorting)
{
	queue *q;

	q = (queue *)malloc (sizeof (queue));
	if (q == NULL) return (NULL);
	memset(q->item,0,QUEUESIZE*sizeof(order));
	q->empty = 1;
	q->full = 0;
	q->head = 0;
	q->tail = 0;
	q->size = 0;
	q->shorting = shorting;
	q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init (q->mut, NULL);
	q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notFull, NULL);
	q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notEmpty, NULL);

	return (q);
}

// ****************************************************************
void queueDelete (queue *q) {
	pthread_mutex_destroy (q->mut);
	free (q->mut);
	pthread_cond_destroy (q->notFull);
	free (q->notFull);
	pthread_cond_destroy (q->notEmpty);
	free (q->notEmpty);
	free (q);
}

// ****************************************************************
void queueAdd (queue *q, order in)
{
	q->item[q->tail] = in;
	q->tail++;
	if (q->tail == QUEUESIZE)
		q->tail = 0;
	if (q->tail == q->head)
		q->full = 1;
	q->empty = 0;
	q->size++;
	return;
}

// ****************************************************************
void queueDel (queue *q, order *out)
{
	*out = q->item[q->head];

	q->head++;
	if (q->head == QUEUESIZE)
		q->head = 0;
	if (q->head == q->tail)
		q->empty = 1;
	q->full = 0;
	q->size--;
	return;
}

// *****************************************************************
void qSortAdd (queue *q,order ord){
	
	queueAdd(q,ord);
	queueSort(q);

}

// *****************************************************************
void queueSort(queue *q){
	order unsortdOrder,tmp;
	char done = 0;
	int tail = q->tail,lasttail;
	if(tail == 0) tail = QUEUESIZE - 1; else tail -= 1;
	lasttail = tail;
	unsortdOrder = q->item[tail];
	while(tail != q->head && done == 0){
		if(tail == 0) tail = QUEUESIZE - 1; else tail = tail - 1;
		if(((q->item[ tail ].price1 > unsortdOrder.price1) && (q->shorting == ASC)) || ((q->item[ tail ].price1 < unsortdOrder.price1) && (q->shorting == DESC))){
			tmp = q->item[tail];
			q->item[tail] = q->item[lasttail];
			q->item[lasttail] = tmp;
		} else {
			done = 1;
		}
		lasttail = tail;
	}	
}

// *****************************************************************
void qSafeSortAdd(queue *l,order ord) {
	/* Lock the list mutex */
	pthread_mutex_lock(l->mut);
	
	/* Check if the list is full. If so, wait on the notFull condition variable */
	while(l->full){
		pthread_cond_wait(l->notFull,l->mut);
	}
	
	/* Insert the new order, and sort the queue */
	qSortAdd(l,ord);
	
	/* Broadcast that the queue isn't empty */
	pthread_cond_broadcast(l->notEmpty);
	
	/* Unlock the list mutex and return */
	pthread_mutex_unlock(l->mut);
}

// *****************************************************************
void qSafeAdd(queue *q,order arg) {
	pthread_mutex_lock (q->mut);
	while (q->full) {
		pthread_cond_wait (q->notFull, q->mut);
	}
	queueAdd(q, arg);
	pthread_cond_broadcast(q->notEmpty);
	pthread_mutex_unlock(q->mut);
}

// *****************************************************************
void qSafeDelete(queue *q,order *arg) {
	
	pthread_mutex_lock(q->mut);
	while (q->empty){
		pthread_cond_wait(q->notEmpty, q->mut);
	}
	queueDel(q, arg);
	
	pthread_cond_broadcast(q->notFull);
	pthread_mutex_unlock(q->mut);
	
}

// *****************************************************************
/* Returns the the order at the head of the queue */
order *qGetFirst(queue *q) {
	return &(q->item[q->head]);
}
