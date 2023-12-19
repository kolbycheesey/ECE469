#include "shared.h"
#include "usertraps.h"

void initQueue(Queue *q) {
  int i;
  q->head = 1;
  q->tail = 0;
  q->size = 0;
  for (i = 0; i < HELLO_WORLD_LEN; i++) {
    q->buf[i] = 0;
  }
}

void enqueue(Queue *q, char c) {
  if (q->size == HELLO_WORLD_LEN) { //full
    Printf("Queue Full!");
  }
  else {
    q->buf[q->head] = c;
    q->head = (q->head + 1) % HELLO_WORLD_LEN;
    q->size++;
  }
}

char dequeue(Queue *q) {
  char c;
  if (q->size == 0) {
    Printf("Queue empty!");
    return 0;
  }
  c = q->buf[q->tail];
  q->buf[q->tail] = 0;
  q->tail = (q->tail + 1) % HELLO_WORLD_LEN;
  q->size--;
  return c;
}
