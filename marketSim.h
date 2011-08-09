#ifndef MARKET_SIM_H
#define MARKET_SIM_H

#include <pthread.h>
#include <stdio.h>

#define QUEUESIZE 5000

#define ASC 101

#define DESC 100

#define ABV 111

#define BLW 110

typedef struct {
	long id,oldid;
	long timestamp;
	int vol;
	int price1, price2;
	char action, type;
} order;


typedef struct {
	order ord;
	void *next;
} order_t;

typedef struct {
	order_t *HEAD;
	unsigned int size;
	unsigned int MAX_SIZE;
	int shorting, full, empty, price_above, price_below, signal_type;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty, *notAbove, *notBelow;
} llist;

typedef struct {
	order item[QUEUESIZE];
	int full, empty, shorting, head, tail, size;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} queue;

extern int currentPriceX10;

extern pthread_mutex_t *price_mut;

extern pthread_cond_t *cur_price_changed;

extern FILE *log_file;

void *Prod (void *q);

void *Cons (void *q);

void inputConsumer(queue *q);

order makeOrder();

long getTimestamp();

void dispOrder(order ord);

queue *queueInit (int shorting);

void queueDelete (queue *q);

void queueAdd (queue *q, order ord);

void qSortAdd (queue *q,order ord);

void queueSort(queue *q);

void queueDel (queue *q, order *ord);

void llistAdd (llist *l, order ord, order_t *after);

void llistDel ( llist *l, order* ord);

order_t *llistInsertHere( llist *l, order ord);

llist *llistInit(int shorting,int singal_type);

#endif
