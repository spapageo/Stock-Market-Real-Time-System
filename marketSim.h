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
	long int id, oldid, timestamp;
	short int vol;
	short int price1, price2;
	char action, type;
} order;


typedef struct {
	order ord;
	void *next;
} order_t;

typedef struct {
	order_t *HEAD;
	int size, MAX_SIZE;
	char shorting, full, empty;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty, *notAbove, *notBelow;
} llist;

typedef struct {
	order item[QUEUESIZE];
	long head, tail, size;
	int full, empty;
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

queue *queueInit (void);

void queueDelete (queue *q);

void queueAdd (queue *q, order ord);

void queueDel (queue *q, order *ord);

void llistAdd (llist *l, order ord, order_t *after);

void llistDel ( llist *l, order* ord);

order_t *llistInsertHere( llist *l, order ord);

llist *llistInit(int shorting);

#endif
