/**
 * 
 * Copyright (c) 2011 Spyridwn Papageorgiou
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * *The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **/

#ifndef MARKET_SIM_H
#define MARKET_SIM_H

#include <pthread.h>
#include <stdio.h>

#define QUEUESIZE 5000

#define ASC 101

#define DESC 100

typedef struct {
	long int timestamp;
	int vol, id, oldid, price1, price2;
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
	pthread_mutex_t *mut;
} rec;

typedef struct {
	pthread_mutex_t *mut;
	pthread_cond_t *cond;
	char trigger;
} signal;

extern rec saves;

extern int currentPriceX10;

extern pthread_mutex_t *price_mut;

extern FILE *log_file;

extern signal *slimit;

extern signal *lim;

signal * signalInit();

void signalSend(signal *s);

void signalWait(signal *s);

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

char queueSearchDelete(queue *q,int id);

#endif

