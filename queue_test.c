#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "signal.h"

#define SIGNAL_QUEUE_SIZE 10

//------------------------------------------------------------------------------

struct queue_node {
  struct signal_t* data;
  struct queue_node* next;
};

//------------------------------------------------------------------------------

struct signal_queue {
  struct queue_node* front;
  struct queue_node* rear;

  unsigned long size;
};

//------------------------------------------------------------------------------

struct signal_queue*
q_new()
{
  struct signal_queue* temp = (struct signal_queue*)malloc(sizeof(struct signal_queue));
  temp->front = NULL;
  temp->rear = NULL;

  return temp;
}

//------------------------------------------------------------------------------

void
q_enqueue(struct signal_queue* queue, struct signal_t* sig)
{
  if (queue->size >= SIGNAL_QUEUE_SIZE)
    return;

  struct queue_node* temp = (struct queue_node*)malloc(sizeof(*queue->front));
  temp->data = sig;
  temp->next = NULL;

  ++queue->size;

  if (queue->front == NULL &&  queue->rear == NULL)
  { // empty queue
    queue->front = queue->rear = temp;
    return;
  }

  queue->rear->next = temp;
  queue->rear = temp;
}

//------------------------------------------------------------------------------

void
q_dequeue(struct signal_queue* queue) {
  struct queue_node* temp = queue->front;
  if (queue->front == NULL)
  { // queue is empty
    return;
  }

  if (queue->front == queue->rear)
  {
    queue->front = queue->rear = NULL;
  }
  else
  {
    queue->front = queue->front->next;
  }

  --queue->size;
  free(temp->data);
  free(temp);
}

//------------------------------------------------------------------------------

struct signal_t*
q_front(struct signal_queue* queue)
{
  if (queue->front == NULL)
  { // queue is empty
    return NULL;
  }

  return queue->front->data;
}

//------------------------------------------------------------------------------

void
q_clear(struct signal_queue* queue)
{
  while (q_front(queue))
    q_dequeue(queue);
}

//------------------------------------------------------------------------------

void print_signal(struct signal_t* s)
{
  printf("id: %d, time: %lu, data: %s\n", s->id, s->timestamp, s->data);
}

//------------------------------------------------------------------------------

void print_queue(struct signal_queue* queue)
{
  struct queue_node* temp = queue->front;

  printf("Queue size: %lu\n", queue->size);

  while (temp) {
    print_signal(temp->data);
    temp = temp->next;
  }
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  int i = 0;
  int entries = 12;

  struct signal_queue* queue = q_new();

  for (i=0; i<entries; ++i) {
    struct signal_t* s = (struct signal_t*)malloc(sizeof(struct signal_t*));
    s->id = i;
    s->timestamp = i*100;
    snprintf(s->data, 8, "%d%d%d", i, i, i);

    q_enqueue(queue, s);
  }

  print_queue(queue);
  q_dequeue(queue);
  print_queue(queue);

  q_clear(queue);
  free(queue);
}
