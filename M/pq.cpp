#include "PriorityQueue.h"


int main()
{
  PriorityQueue* PQ = PQ_Create(2);
  PQNode MinNode;
  int i;
  
  PQNode Node[7] = 
  {
    {54, (void*) "이성과 데이트"},
    {157,(void*) "야동감상"},
    {100, (void*) "게임하기"},
    {10, (void*) "공부"},
    {32, (void*) "밥먹기"},
    {20, (void*) "돈벌기"},
    {50,(void*) "친구와 술먹기"}    
  };

  for(i=0; i<7; i++)
    PQ_Enqueue(PQ, Node[i]);

  // Priority Queue
  // {10, 32, 20, 157, 54, 100, 50}

  printf(" Priorty Queue에 존재하는 작업의 수 : %d\n", PQ->UsedSize);


  while( !PQ_IsEmpty(PQ) )
  {
    PQ_Dequeue(PQ, &MinNode);
    printf(" Work : %s (Priority : %d)\n", MinNode.Data , MinNode.Priority);
  }
  
  PQ_Destroy(PQ);
  return 0;
}
