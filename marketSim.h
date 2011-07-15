#ifndef MARKET_SIM_H
#define MARKET_SIM_H

#include <pthread.h>

#define QUEUESIZE 5000

typedef struct {
	long id,oldid;
	long timestamp;
	int vol;
	int price1, price2;
	char action, type;
} order;

typedef struct {
	order item[QUEUESIZE];
	long head, tail;
	int full, empty;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} queue;

void *Prod (void *q);

void *Cons (void *q);

void inputConsumer(queue *q);

order makeOrder();

inline long getTimestamp();

void dispOrder(order ord);

int currentPriceX10;

queue *queueInit (void);

void queueDelete (queue *q);

void queueAdd (queue *q, order ord);

void queueDel (queue *q, order *ord);


#endif
