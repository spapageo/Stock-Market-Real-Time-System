/*
 *      A Stock Market Simulator Skeleton
 *      by Nikos Pitsianis
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
#include <pthread.h>
#include "marketSim.h"

// ****************************************************************
int main() {

  // reset number generator seed
  // srand(time(NULL) + getpid());
  srand(0); // to get the same sequence

  // start the time for timestamps
  gettimeofday (&startwtime, NULL);

  pthread_t prod, cons;
  queue *q = queueInit();

  pthread_mutex_init(&mut, NULL);

  pthread_create(&prod, NULL, Prod, q);
  pthread_create(&cons, NULL, Cons, q);

  // I actually do not expect them to ever terminate
  pthread_join(prod, NULL);
  pthread_join(cons, NULL);

  pthread_exit(NULL);
}

// ****************************************************************
void *Prod (void *arg){
  queue *q = (queue *) arg;
  int i;

  // for (i=0; i<QUEUESIZE; i++){
  while (1) {
    pthread_mutex_lock (q->mut);
    while (q->full) {
      printf ("*** Incoming Order Queue is FULL.\n"); fflush(stdout);
      pthread_cond_wait (q->notFull, q->mut);
    }
    queueAdd (q, makeOrder());
    pthread_mutex_unlock (q->mut);
    pthread_cond_signal (q->notEmpty);
  }
}

// ****************************************************************
void *Cons (void *arg){
  queue *q = (queue *) arg;
  int i;
  order ord;

  //for (i=0; i<QUEUESIZE; i++){
  while (1) {
    pthread_mutex_lock (q->mut);
    while (q->empty) {
      printf ("*** Incoming Order Queue is EMPTY.\n"); fflush(stdout);
      pthread_cond_wait (q->notEmpty, q->mut);
    }
    queueDel (q, &ord);
    pthread_mutex_unlock (q->mut);
    pthread_cond_signal (q->notFull);

    // YOUR CODE IS CALLED FROM HERE
    // Process that order!
    printf ("Processing at time %8d : ", getTimestamp());
    dispOrder(ord); fflush(stdout);

  }
}

// ****************************************************************
order makeOrder(){

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
  if (u2 < 0.2){
    ord.type = 'M';                 // Market order
    ord.vol = (1 + rand()%50)*100;

  }else if (0.2 <= u2 && u2 < 0.4){
    ord.type = 'L';                 // Limit order
    ord.vol = (1 + rand()%50)*100;
    ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));

  }else if (0.4 <= u2 && u2 < 0.6){
    ord.type = 'S';                 // Stop order
    ord.vol = (1 + rand()%50)*100;
    ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));

  }else if (0.6 <= u2 && u2 < 0.8){
    ord.type = 'T';                 // Stop Limit order
    ord.vol = (1 + rand()%50)*100;
    ord.price1 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));
    ord.price2 = currentPriceX10 + 10*(0.5 -((double)rand()/(double)RAND_MAX));

  }else if (0.8 <= u2){
    ord.type = 'C';                 // Cancel order
    ord.oldid = ((double)rand()/(double)RAND_MAX)*count;
  }

  // dispOrder(ord);

  return (ord);
}

// ****************************************************************
inline long getTimestamp(){

  gettimeofday(&endwtime, NULL);

  return((double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
		  + endwtime.tv_sec - startwtime.tv_sec)*1000);
}

// ****************************************************************
void dispOrder(order ord){

  printf("%08d ", ord.id);
  printf("%08d ", ord.timestamp);
  switch( ord.type ) {
  case 'M':
    printf("%c ", ord.action);
    printf("Market (%4d)        ", ord.vol); break;
  case 'L':
    printf("%c ", ord.action);
    printf("Limit  (%4d,%5.1f) ", ord.vol, (float) ord.price1/10.0); break;
  case 'S':
    printf("%c ", ord.action);
    printf("Stop   (%4d,%5.1f) ", ord.vol, (float) ord.price1/10.0); break;
  case 'T':
    printf("%c ", ord.action);
    printf("StopLim(%4d,%5.1f,%5.1f) ",
	   ord.vol, (float) ord.price1/10.0, (float) ord.price2/10.0); break;
  case 'C':
    printf("* Cancel  %6d        ", ord.oldid); break;
  default : break;
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
void queueDelete (queue *q)
{
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
