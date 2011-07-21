#ifndef MARKET_SIM_H
#define MARKET_SIM_H

#include <pthread.h>
#include <stdio.h>

#define QUEUESIZE 15000

#define ASC 101

#define DESC 100

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
	int shorting;
	int full, empty;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} llist;

typedef struct {
	order item[QUEUESIZE];
	long head, tail;
	int full, empty;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} queue;

int currentPriceX10;

pthread_mutex_t *price_mut;

FILE *log_file;

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

order_t *llistInsert( llist *l, order ord);

llist *llistInit(int shorting);

#endif
