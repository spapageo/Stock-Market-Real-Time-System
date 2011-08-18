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
#include "cancel.h"


// *****************************************************************
struct timeval startwtime, endwtime;

int currentPriceX10;

rec saves;

signal *slimit;

signal *lim;

pthread_mutex_t *price_mut;

pthread_cond_t *price_changed;

FILE *log_file;

// ****************************************************************
int main() {

	/* reset number generator seed */
	srand(time(NULL) + getpid());
	//srand(0); // to get the same sequence
	
	/* initialize the price and it's mutex */
	currentPriceX10 = 1000;
	price_mut = malloc (sizeof(pthread_mutex_t));
	pthread_mutex_init(price_mut,NULL);
	
	slimit = signalInit();
	lim = signalInit();
	
	//open the log file for writing
	log_file = fopen("logfile.txt","w+");

	saves.mut = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(saves.mut,NULL);
	saves.archive = calloc(INT_MAX,sizeof(char));
	if(saves.archive == NULL) exit(0);
	
	// start the time for timestamps
	gettimeofday (&startwtime, NULL);
	
	//Create all the thread variables
	pthread_t prod, cons, cons2, market, lim_thread, stop_thread, slim_thread, cancel;
	
	// Initialize the incoming order queue
	queue *q = queueInit(0);
	
	// Initialize the buy and sell market queues.
	msq = queueInit(0);
	mbq = queueInit(0);

	// Initialize the buy and sell limit order queues
	lsq = queueInit(ASC);
	lbq = queueInit(DESC);
	
	// Initialize the buy and sell stop order queues
	ssq = queueInit(ASC);
	sbq = queueInit(DESC);
	
	// Initialize the buy and sell stop limit order queues
	tsq = queueInit(ASC);
	tbq = queueInit(DESC);

	//Initialize the cancel queue
	cq = queueInit(0);
	
	printf("     buffer |market sell|market buy|limit sell|limit buy |stop sell | stop buy |slimit sell|slimit buy");
	printf("\n\n");
	
	// Create and launch all the appropriate threads
	pthread_create(&prod, NULL, Prod, q);
	pthread_create(&cons, NULL, Cons, q);
	pthread_create(&cons2, NULL, Cons, q);
	pthread_create(&lim_thread, NULL, limitWorker, NULL);
	pthread_create(&market, NULL, marketWorker, NULL);
	pthread_create(&stop_thread, NULL, stopWorker, NULL);
	pthread_create(&slim_thread, NULL, stoplimitWorker, NULL);
	pthread_create(&cancel, NULL, cancelWorker, NULL);
	
	
	// I actually do not expect them to ever terminate
	pthread_join(prod, NULL);
	
	
	pthread_exit(NULL);
}

// ****************************************************************
void *Prod (void *arg) {
	queue *q = (queue *) arg;
	while (1) {
		
		
		fputs("\033[A\033[2K",stdout);
		rewind(stdout);
		ftruncate(1,0);
		printf("**** %05d **** %05d **** %05d **** %05d **** %05d **** %05d **** %05d **** %05d **** %05d ****\n",q->size,msq->size,mbq->size,lsq->size,lbq->size,ssq->size,sbq->size,tsq->size,tbq->size);
		fflush(stdout);
		
		pthread_mutex_lock (q->mut);
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
		
		pthread_mutex_lock(saves.mut);
		if (ord.type == 'M') {
			if(ord.action == 'S'){
				saves.archive[ord.id] = 'M';
				pthread_mutex_unlock(saves.mut);
				qSafeAdd(msq,ord);
			} else {
				saves.archive[ord.id] = 'N';
				pthread_mutex_unlock(saves.mut);
				qSafeAdd(mbq,ord);
			}
		} else if (ord.type == 'L') {
			if(ord.action == 'S'){
				saves.archive[ord.id] = 'L';
				pthread_mutex_unlock(saves.mut);
				qSafeSortAdd(lsq,ord);
			}else{
				saves.archive[ord.id] = 'K';
				pthread_mutex_unlock(saves.mut);
				qSafeSortAdd(lbq,ord);
			}
		} else if (ord.type == 'S') {
			if(ord.action == 'S'){
				saves.archive[ord.id] = 'S';
				pthread_mutex_unlock(saves.mut);
				qSafeSortAdd(ssq,ord);
			}else{
				saves.archive[ord.id] = 'P';
				pthread_mutex_unlock(saves.mut);
				qSafeSortAdd(sbq,ord);
			}
		} else if (ord.type == 'T') {
			if(ord.action == 'S'){
				saves.archive[ord.id] = 'T';
				pthread_mutex_unlock(saves.mut);
				qSafeSortAdd(tsq,ord);
			}else{
				saves.archive[ord.id] = 'W';
				pthread_mutex_unlock(saves.mut);
				qSafeSortAdd(tbq,ord);
			}
		} else {
			saves.archive[ord.id] = 'C';
			pthread_mutex_unlock(saves.mut);
			qSafeAdd(cq,ord);
			
		}


	}
	return NULL;
}

// ****************************************************************
order makeOrder() {
	//pthread_mutex_lock(price_mut);
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
	ord.vol = 0;

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
		if(ord.price1 < 0) ord.price1 = -ord.price1;
		ord.price2 = 0;

	} else if (0.4 <= u2 && u2 < 0.6) {
		ord.type = 'S';				 // Stop order
		ord.vol = (1 + rand()%50)*100;
		ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));
		if(ord.price1 < 0) ord.price1 = -ord.price1;
		ord.price2 = 0;
	} else if (0.6 <= u2 && u2 < 0.8) {
		ord.type = 'T';				 // Stop Limit order
		ord.vol = (1 + rand()%50)*100;
		ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));
		if(ord.price1 < 0) ord.price1 = -ord.price1;
		ord.price2 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));
		if(ord.price2 < 0) ord.price2 = -ord.price1;

	} else {
		ord.type = 'C';				 // Cancel order
		ord.oldid = ((double)rand()/(double)RAND_MAX)*count;
		ord.price1 = 0;
		ord.price2 = 0;
	}
	//pthread_mutex_unlock(price_mut);
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
	if (q->tail == QUEUESIZE - 1)
		q->tail = 0;
	else
		q->tail++;
	
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

	
	if (q->head == QUEUESIZE - 1)
		q->head = 0;
	else
		q->head++;
	
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

// *****************************************************************
signal* signalInit() {
	signal *s = malloc(sizeof(signal));
	s->mut = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(s->mut,NULL);
	s->cond = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(s->cond,NULL);
	s->trigger = 1;
	return s;
}

// *****************************************************************
void signalWait(signal* s) {
	pthread_mutex_lock(s->mut);
	s->trigger = 1;
	while(s->trigger){
		pthread_cond_wait(s->cond,s->mut);
	}
	pthread_mutex_unlock(s->mut);
}

// *****************************************************************
void signalSend(signal* s) {
	pthread_mutex_lock(s->mut);
	s->trigger = 0;
	pthread_cond_broadcast(s->cond);
	pthread_mutex_unlock(s->mut);
}


// *****************************************************************
char queueSearchDelete(queue *q,int id){
	int tail;
	int lasttail;
	char found = 0;

	if(q->empty == 1) return -1;
	
	tail = q->tail;

	/* Perform the search for the order with the specified id */
	do {
		if(tail == 0) tail = QUEUESIZE - 1; else tail--; //decreament the tail
		if (q->item[tail].id == id)	found = 1;
	} while( (tail != q->head) && (found == 0));

	/*
	 * If the order was found perform the move
	 * of all the elements one spot to the left to fill the gap
	 */
	if(found == 1){
		lasttail = tail;
		while(tail != q->tail){
			if(tail == QUEUESIZE - 1) tail = 0; else tail++; //increment the tail
			q->item[lasttail] = q->item[tail];
			lasttail = tail;
		}

		if (q->tail == 0)	q->tail = QUEUESIZE - 1; else	q->tail--; // decrement the tail;

		if (q->head == q->tail)
			q->empty = 1;
		
		q->full = 0;
		q->size--;
		
		return 0;
	} else {
		return -1;
	}
}
