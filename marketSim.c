#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "marketSim.h"
#include "market.h"
#include "limit.h"
#include "stop.h"
#include "stoplimit.h"


//*****************************************************************
struct timeval startwtime, endwtime;

// ****************************************************************
int main() {

	// reset number generator seed
	// srand(time(NULL) + getpid());
	srand(0); // to get the same sequence
	
	//initialize the price and its mutex
	currentPriceX10 = 1000;
	price_mut = malloc (sizeof(pthread_mutex_t));
	pthread_mutex_init(price_mut,NULL);

	//open the log file foe writing
	log_file = fopen("logfile.txt","w+");
	
	// start the time for timestamps
	gettimeofday (&startwtime, NULL);

	pthread_t prod, cons, cons2, market, limit;
	// Initialize the incoming order queue
	queue *q = queueInit();
	
	// Initialize the buy and sell market queues.
	msq = queueInit();
	mbq = queueInit();

	// Initialize the buy and sell limit order lists
	lsl = llistInit(ASC);
	lbl = llistInit(DESC);
	
	// Initialize the buy and sell stop order lists
	ssl = llistInit(ASC);
	sbl = llistInit(DESC);

	// Initialize the buy and sell stop limit order lists
	tsl = llistInit(ASC);
	tbl = llistInit(DESC);

	// Laucnch all the appropriate threads
	pthread_create(&prod, NULL, Prod, q);
	pthread_create(&cons, NULL, Cons, q);
	pthread_create(&cons2, NULL, Cons, q);
	pthread_create(&limit, NULL, limitWorker, q);
	pthread_create(&market, NULL, marketWorker, q);
	
	
	// I actually do not expect them to ever terminate
	pthread_join(limit, NULL);
	pthread_join(prod, NULL);
	pthread_join(cons, NULL);
	pthread_join(cons2, NULL);
	pthread_join(market, NULL);
	

	pthread_exit(NULL);
}

// ****************************************************************
void *Prod (void *arg) {
	queue *q = (queue *) arg;
// 	int size;
	while (1) {
		pthread_mutex_lock (q->mut);
// 		if(q->tail < q->head){
// 			size = QUEUESIZE - q->head +q->tail;
// 		} else {
// 			size = q->tail - q->head;
// 		}
// 		fputs("\033[A\033[2K",stdout);
// 		rewind(stdout);
// 		ftruncate(1,0);
// 		printf("**** %05d ****\n",size);fflush(stdout);
		while (q->full) {
			printf("Incoming order queue is FULL\n");
			fflush(stdout);
			pthread_cond_wait (q->notFull, q->mut);
		}
		queueAdd (q, makeOrder());
		pthread_cond_signal (q->notEmpty);
		pthread_mutex_unlock (q->mut);
	}
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

		pthread_cond_signal (q->notFull);
		pthread_mutex_unlock (q->mut);
		
		// YOUR CODE IS CALLED FROM HERE
		// Process that order!
		if (ord.type == 'M') {
			if(ord.action == 'S')
				qSafeAdd(msq,ord);
			else
				qSafeAdd(mbq,ord);
		} else if (ord.type == 'L') {
			if(ord.action == 'S')
				lSafeAdd(lsl,ord);
			else
				lSafeAdd(lbl,ord);
		} else if (ord.type == 'S') {
			if(ord.action == 'S')
				lSafeAdd(ssl,ord);
			else
				lSafeAdd(sbl,ord);
		} else if (ord.type == 'T') {
			if(ord.action == 'S')
				lSafeAdd(tsl,ord);
			else
				lSafeAdd(tbl,ord);
		} else {
			//Here we hadle the cancel commands
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

	} else if (0.8 <= u2) {
		ord.type = 'C';				 // Cancel order
		ord.oldid = ((double)rand()/(double)RAND_MAX)*count;
		ord.price1 = 0;
		ord.price2 = 0;
	}

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
	if(ord==NULL){printf("Malloc error\n");fflush(stdout);exit(1);}
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
	if (l == NULL) return (NULL);
	l->HEAD = NULL;
	l->MAX_SIZE = QUEUESIZE;
	l->size=0;
	l->shorting = shorting;
	l->empty = 1;
	l->full = 0;
	l->mut = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(l->mut,NULL);
	l->notEmpty = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(l->notEmpty,NULL);
	l->notFull = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(l->notFull,NULL);
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
