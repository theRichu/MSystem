#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

//#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct tagPQNode
{
  int Priority; // 우선순위
  int Type;
  unsigned long Timestamp;
} PQNode;

typedef struct tagPriorityQueue
{

  PQNode* Nodes; // Node를 담는 배열
  int Capacity;
  int UsedSize;

} PriorityQueue;

//////////// 함수 정의 ///////////////

PriorityQueue*  PQ_Create(int Init);

void      PQ_Destroy(PriorityQueue* PQ);

void      PQ_Enqueue(PriorityQueue* PQ, PQNode NewNode);

void      PQ_Dequeue(PriorityQueue* PQ, PQNode* Root);

int       PQ_GetParent(int Index);

int       PQ_GetLeftChild(int Index);

void      PQ_SwapNodes(PriorityQueue* PQ, int Index1, int Index2);

int       PQ_IsEmpty(PriorityQueue* PQ);

#endif
