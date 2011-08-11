#ifndef MARKET_SIM_H
#define MARKET_SIM_H

#include <limits.h>
#include <pthread.h>
#include <stdio.h>

#define QUEUESIZE 5000

#define ASC 101

#define DESC 100



typedef struct {
	long int id, oldid, timestamp;
	int vol;
	int price1, price2;
	char action, type;
} order;


typedef struct {
	order item[QUEUESIZE];
	char full, empty, shorting;
	int head, tail, size;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} queue;

typedef struct {
	char *archive;
} rec;

extern int currentPriceX10;

extern pthread_mutex_t *price_mut;

extern FILE *log_file;

void *Prod (void *q);

void *Cons (void *q);

order makeOrder();

long getTimestamp();

void dispOrder(order ord);

queue *queueInit (int shorting);

void queueDelete (queue *q);

void queueAdd (queue *q, order ord);

order *qGetFirst(queue *q);

void qSortAdd (queue *q,order ord);

void qSafeSortAdd(queue *q,order ord);

void qSafeAdd(queue *q,order arg);

void qSafeDelete(queue *q,order *arg);

void queueSort(queue *q);

void queueDel (queue *q, order *ord);

#endif
