#ifndef SHARED_H
#define SHARED_H

#define HELLO_WORLD_LEN 11
typedef struct queue{
  char buf[HELLO_WORLD_LEN];
  int head;
  int tail;
  int size;
} Queue;

typedef struct data{
  Queue q;
  int numprocs;
}Data;

void enqueue(Queue* q, char data);
char dequeue(Queue *q);
void initQueue(Queue *q);
#endif
