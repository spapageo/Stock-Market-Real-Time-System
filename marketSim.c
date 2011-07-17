/*
 *	  A Stock Market Simulator Skeleton
 *	  by Nikos Pitsianis
 *
 *	Based upon the pc.c producer-consumer demo by Andrae Muys
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include "marketSim.h"
#include "market.h"
#include "limit.h"

//*****************************************************************
struct timeval startwtime, endwtime;

// ****************************************************************
int main()
{

	// reset number generator seed
	// srand(time(NULL) + getpid());
	srand(0); // to get the same sequence
	
	//initialize the price
	currentPriceX10 = 1000;
	
	// start the time for timestamps
	gettimeofday (&startwtime, NULL);

	pthread_t prod, cons, market;
	
	// Initialize the incoming order queue
	queue *q = queueInit();
	
	// Initialize the buy and sell market queues.
	msq = queueInit();
	mbq = queueInit();

	// Initialize the buy and sell limit lists
	lsl = llistInit(ASC);
	lbl = llistInit(DESC);
	
	// Laucnch all the appropriate threads
	pthread_create(&prod, NULL, Prod, q);
	pthread_create(&cons, NULL, Cons, q);
	pthread_create(&market, NULL, marketWorker, q);
	
	// I actually do not expect them to ever terminate
	pthread_join(prod, NULL);
	pthread_join(cons, NULL);
	pthread_join(market, NULL);

	pthread_exit(NULL);
}

// ****************************************************************
void *Prod (void *arg) {
	queue *q = (queue *) arg;
	while (1) {
		pthread_mutex_lock (q->mut);
		while (q->full) {
			printf ("*** Incoming Order Queue is FULL.\n");
			fflush(stdout);
			pthread_cond_wait (q->notFull, q->mut);
		}
		queueAdd (q, makeOrder());
		pthread_mutex_unlock (q->mut);
		pthread_cond_signal (q->notEmpty);
	}
}

// ****************************************************************
void *Cons (void *arg) {
	queue *q = (queue *) arg;
	order ord;

	while (1) {
		pthread_mutex_lock (q->mut);
		while (q->empty) {
			printf ("*** Incoming Order Queue is EMPTY.\n");
			fflush(stdout);
			pthread_cond_wait (q->notEmpty, q->mut);
		}
		queueDel (q, &ord);

		pthread_mutex_unlock (q->mut);
		pthread_cond_signal (q->notFull);

		// YOUR CODE IS CALLED FROM HERE
		// Process that order!
		if (ord.type == 'M') {
			if(ord.action == 'S')
				mQueueAdd(msq,ord);
			else
				mQueueAdd(mbq,ord);
		} else if (ord.type == 'L') {
			if(ord.action == 'S'){
				dispOrder(ord);
				fflush(stdout);
				lSafeAdd(lsl,ord);
			} else
				lSafeAdd(lbl,ord);
		}
		//dispOrder(ord);
		//fflush(stdout);
		//printf ("Processing at time %8d : ", getTimestamp());


	}
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
	ord.timestamp = getTimestamp();

	// Buy or Sell
	ord.action = ((double)rand()/(double)RAND_MAX <= 0.5) ? 'B' : 'S';

	// Order type
	double u2 = ((double)rand()/(double)RAND_MAX);
	if (u2 < 0.2) {
		ord.type = 'M';				 // Market order
		ord.vol = (1 + rand()%50)*100;

	} else if (0.2 <= u2 && u2 < 0.4) {
		ord.type = 'L';				 // Limit order
		ord.vol = (1 + rand()%50)*100;
		ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));

	} else if (0.4 <= u2 && u2 < 0.6) {
		ord.type = 'S';				 // Stop order
		ord.vol = (1 + rand()%50)*100;
		ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));

	} else if (0.6 <= u2 && u2 < 0.8) {
		ord.type = 'T';				 // Stop Limit order
		ord.vol = (1 + rand()%50)*100;
		ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));
		ord.price2 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));

	} else if (0.8 <= u2) {
		ord.type = 'C';				 // Cancel order
		ord.oldid = ((double)rand()/(double)RAND_MAX)*count;
	}

	// dispOrder(ord);

	return (ord);
}

// ****************************************************************
inline long getTimestamp() {

	gettimeofday(&endwtime, NULL);

	return((double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
					+ endwtime.tv_sec - startwtime.tv_sec)*1000);
}

// ****************************************************************
void dispOrder(order ord) {

	printf("%08ld ", ord.id);
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
		printf("* Cancel  %6ld		", ord.oldid);
		break;
	default :
		break;
	}
	printf("\n");
}

// ****************************************************************
// ****************************************************************
queue *queueInit (void)
{
	queue *q;

	q = (queue *)malloc (sizeof (queue));
	if (q == NULL) return (NULL);

	q->empty = 1;
	q->full = 0;
	q->head = 0;
	q->tail = 0;
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

	return;
}

//***************************************************************
void llistAdd(llist *l,order o,order_t *after){
	if(l->empty == 1){
		l->empty = 0;
	} else if (l->size == l->MAX_SIZE - 1){
		l->full = 1;
	}
	order_t *ord = malloc(sizeof(order_t));
	ord->ord = o;
	l->size += 1;
	
	if(after != NULL){
		ord->next = after->next;
		after->next = ord;
	} else {
		ord->next = l->HEAD;
		l->HEAD = ord;
	}
}

//***************************************************************
void llistDel(llist *l,order *o){
	if(l->size == 1){
		l->empty = 1;
	} else if (l->size == l->MAX_SIZE) {
		l->full = 0;
	}
	l->size -= 1;
	order_t *ord = l->HEAD;
	l->HEAD = ord->next;
	*o = ord->ord;
	free(ord);
}

//***************************************************************
llist *llistInit(int shorting){
	llist *l = malloc(sizeof(llist));
	l->HEAD = NULL;
	l->MAX_SIZE = QUEUESIZE;
	l->size=0;
	l->shorting = shorting;
	l->empty = 1;
	l->full = 0;
	l->mut = malloc(sizeof(pthread_mutex_t));
	l->notEmpty = malloc(sizeof(pthread_cond_t));
	l->notFull = malloc(sizeof(pthread_cond_t));
	return l;
}

//****************************************************************
order_t *llistInsert( llist *l, order ord){
	int i,value = ord.price1;
	if ( l->size == 0 ){
		return NULL;
	}
	order_t *lo = NULL, *co = l->HEAD;
	if (l->shorting == ASC){
		for (i = 0;i < l->size; i++){
			if ( co->ord.price1 > value ){
				break;
			} else {
				lo = co;
				co = lo->next;
			}
		}
	} else {
		for (i = 0;i < l->size; i++){
			if ( co->ord.price1 < value ){
				break;
			} else {
				lo = co;
				co = lo->next;
			}
		}
	}
	return lo;
}