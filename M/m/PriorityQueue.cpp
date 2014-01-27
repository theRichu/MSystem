
#include "PriorityQueue.h"


// Priority Queue 생성
PriorityQueue* PQ_Create(int Init)
{
  PriorityQueue* NewPQ = (PriorityQueue*) malloc( sizeof(PriorityQueue) );

  NewPQ->Capacity = Init;
  NewPQ->Nodes    = (PQNode*) malloc( sizeof(PQNode) * NewPQ->Capacity );
  NewPQ->UsedSize = 0;

  return NewPQ;
}

// Priority Queue 파괴
void PQ_Destroy(PriorityQueue* PQ)
{
  free(PQ->Nodes);
  free(PQ);
}

// Node를 삽입하는 함수
// 1. Capacity와 UsedSize를 검사후 용량을 늘리는 작업
// 2. Heap 자료구조의 규칙 유지. - Root노드는 최소값, 부모노드는 L,R Child보다 값이 작음.
//   - 새로 삽입할 노드를 배열의 젤 뒤(Leaf노드들중 최 우측)에 삽입후 부모와 값 비교
void PQ_Enqueue(PriorityQueue* PQ, PQNode NewNode)
{
  int CP = PQ->UsedSize ; // Current Position
  int PP = PQ_GetParent(CP); // Parent Position

  if( PQ->UsedSize == PQ->Capacity)
  { // 용량이 가득 찼다면
    if(PQ->Capacity == 0)
      PQ->Capacity = 1;

    PQ->Capacity *= 2; // 자기 자신의 두배 만큼 증가
    PQ->Nodes = (PQNode*) realloc( PQ->Nodes , sizeof(PQNode) * PQ->Capacity );
  }
  // 용량 재조정 작업후 Heap 자료구조 규칙 유지
  PQ->Nodes[CP] = NewNode;

  while( CP > 0 && ( PQ->Nodes[CP].Priority < PQ->Nodes[PP].Priority ) )
  { // CP의 우선순위가 PP의 우선순위 보다 낮은 경우 - CP와 PP를 교환

    PQ_SwapNodes(PQ, CP, PP);

    CP = PP;
    PP = PQ_GetParent(CP);
  }

  (PQ->UsedSize) ++;
}

// Heap의 최소값 삭제에 해당하는 함수
// 1. 최소값(Heap의 Root)를 매개변수 Root에 Copy 한 후
//  - Heap 자료구조 규칙 유지 작업 - Root노드는 최소값, 부모노드는 L,R Child보다 값이 작음.
//  - 배열 젤 뒤(Leaf노드들중 최 우측)의 노드를 Root Node자리에 넣은후 (0번) L,R Child들 중 작은 값과 비교후
//   작다면 자료 교환. (이 때의 값이라는건 우선순위를 말함)
// 2. 삭제 작업이 완료 된 후 UsedSize가 Capacity의 값보다 심하게 작을때(ex) UsedSize < Capacity/2 )
//   용량을 줄이는 작업.
void PQ_Dequeue(PriorityQueue* PQ, PQNode* Root)
{
  int PP  = 0; // Parent Position
  int LCP = 0; // Left Child Position
  int RCP = 0; // Right Child Position

  memcpy(Root, & PQ->Nodes[PP], sizeof(PQNode) );
  memset(& PQ->Nodes[PP], 0, sizeof(PQNode) ); // 삭제되는 Root Node를 초기화

  PQ->UsedSize--; // 최소값 삭제로 Node수 감소

  PQ_SwapNodes(PQ, PP, PQ->UsedSize); // Left노드들중 최 우측을 Root의 위치로 이동후


  // Heap 자료구조 규칙 검사 후 재 구성
  LCP = PQ_GetLeftChild(PP);
  RCP = LCP + 1;

  while(1)
  {
    int SC = 0 ; // Selected Child
    // LC와 RC중 작은 값을 선택후 P와 값을 비교하고 작다면 P와 교환함.

    if(LCP >= PQ->UsedSize)
      break; 
    //  LCP가 PQ_GetLeftChild 함수에 인해 UsedSize보다 큰 값을 가지면 메모리 참조 오류를 
    //발생시키므로 예외처리

    if(RCP >= PQ->UsedSize)
    { // RCP가 UsedSize보다 크다면 LCP(RCP-1)의 부모노드는 LC만 가지고 있다는 뜻이므로
      SC = LCP;
    }
    else // 그 외의 경우 LC와 RC를 비교후 작은 값에 해당하는 index를 SC로
    {
      if( PQ->Nodes[LCP].Priority < PQ->Nodes[RCP].Priority)
        SC = LCP;
      else
        SC = RCP;
    }
    // 선택된 SC와 PP의 우선순위를 비교 후 PP의 우선순위가 크다면 SC와 교환
    if( PQ->Nodes[SC].Priority < PQ->Nodes[PP].Priority )
    {
      PQ_SwapNodes(PQ, PP, SC);
      
      PP = SC;
      // 아래로 타고 내려감.
    }
    else
      break;

    LCP = PQ_GetLeftChild(PP);
    RCP = LCP + 1;
  }

  // 용량 재조정
  if( PQ->UsedSize < (PQ->Capacity / 2) )
  {
    PQ->Capacity /= 2;
    PQ->Nodes = (PQNode*) realloc(PQ->Nodes , sizeof(PQNode) * PQ->Capacity );
  }
}

// Array로 표현된 Heap 자료구조의 특정노드의 부모를 얻으려면.. (Index-1)/2;
int PQ_GetParent(int Index)
{
  return (Index-1)/2;
}

// Array로 표현된 Heap 자료구조의 특정노드의 LeftChild를 얻으려면 (Index*2)+1;
int PQ_GetLeftChild(int Index)
{
  return (Index*2)+1;
}

void PQ_SwapNodes(PriorityQueue* PQ, int Index1, int Index2)
{
  int CS = sizeof(PQNode); // Copy Size
  PQNode* Temp = (PQNode*) malloc( CS );
  
  // swap 구현
  memcpy(Temp, &(PQ->Nodes[Index1]), CS);

  memcpy(&( PQ->Nodes[Index1] ), &( PQ->Nodes[Index2] ), CS);

  memcpy(&( PQ->Nodes[Index2] ), Temp, CS);

  free(Temp);
}

// Heap이 비어있는지 여부를 반환
int PQ_IsEmpty(PriorityQueue* PQ)
{
  return ( PQ->UsedSize == 0 );
}
