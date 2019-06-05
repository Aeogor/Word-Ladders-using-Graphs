/*queue.h*/

//
// Queue:
//
typedef int QueueElementType;
typedef struct Queue
{
  QueueElementType  *Elements;  // array of elements in queue:
  int  Front;        // index of first element in Q
  int  Back;         // index of last element in Q
  int  NumElements;  // # of elements currently in Q
  int  Capacity;     // max # of elements that can fit in Q
} Queue;

Queue *CreateQueue(int N);
void   DeleteQueue(Queue *Q);
int    isEmptyQueue(Queue *Q);
int    Enqueue(Queue *Q, QueueElementType e);
QueueElementType Dequeue(Queue *Q);
int    isElementInQueue(Queue *Q, QueueElementType e);
