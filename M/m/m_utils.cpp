#include "m_utils.h"

/**************************************************************
 * Linked Queue
 ****************************************************************/
void LQ_CreateQueue(LinkedQueue** Queue)
{
  /* 큐를 자유 저장소에 생성 */
  (*Queue) = (LinkedQueue*)malloc(sizeof(LinkedQueue));
  (*Queue)->Front = NULL;
  (*Queue)->Rear = NULL;
  (*Queue)->Count = 0;
}
void LQ_DestoryQueue(LinkedQueue* Queue)
{
  while(!LQ_IsEmpty(Queue))
  {
    Node* Popped = LQ_Dequeue(Queue);
    LQ_DestoryNode(Popped);
  }
  /*  큐를 자유 저장소에서 해제 */
  free(Queue);
}
Node* LQ_CreateNode(int _data, unsigned long _timestamp)
{
  Node* NewNode = (Node*)malloc(sizeof(Node));
  NewNode->Data = _data;  /* 데이터를 저장한다. */
  NewNode->Timestamp = _timestamp;  /* 데이터를 저장한다. */

  NewNode->NextNode = NULL;   /* 다음 노드에 대한 포인터는 NULL로 초기화한다. */

  return NewNode;   /* 노드의 주소를 반환한다. */
}
void LQ_DestoryNode(Node* _Node)
{
  free(_Node);
}
void LQ_Enqueue(LinkedQueue* Queue, Node* NewNode)
{
  if(Queue->Front == NULL)
  {
    Queue->Front = NewNode;
    Queue->Rear = NewNode;
    Queue->Count++;
  }
  else{
    Queue->Rear->NextNode = NewNode;
    Queue->Rear = NewNode;
    Queue->Count++;
  }
}
Node* LQ_Dequeue(LinkedQueue* Queue)
{
  /* LQ_Dequeue() 함수가 반환할 최상위 노드 */
  Node* Front = Queue->Front;

  if(Queue->Front->NextNode == NULL)
  {
    Queue->Front = NULL;
    Queue->Rear = NULL;
  }
  else{
    Queue->Front = Queue->Front->NextNode;
  }
  Queue->Count--;
  return Front;
}
int LQ_IsEmpty(LinkedQueue* Queue)
{
  return (Queue->Front == NULL);
}

/**********************************************************
 * M_Device Class
 ************************************************************/
long previousMillis_start = 0;

void M_Device::setProtocol(int _p, int _r, int _d, int _a){
  mode.code = _p;
  mode.repeat_num = _r;
  mode.delay_time = _d; 
  mode.aftercollecting_time = _a;
}
void M_Device::stateReset(){
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***INITIALIZED");
#endif
  m_times=0;
  state=INITIAL;
}
void M_Device::stateAftercollecting(){
  previousMillis_start = (int)millis();
  state=AFTERCOLLECTING;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***AFTERCOLLECTING");
#endif
}
void M_Device::stateMeasuring(){
  state=MEASURING;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***MEASURING");
  SerialUSB.print(" /protocol: ");
  switch(mode.code){
  case P_UNDEFINED:
    SerialUSB.print("UNDEFINED");
    break;
  case P_NORMAL:
    SerialUSB.print("P_NORMAL");
    break;
  }
  SerialUSB.print(" /repeat_num: ");
  SerialUSB.print(mode.repeat_num);
  SerialUSB.print(" /delay_time: ");
  SerialUSB.print(mode.delay_time);
  SerialUSB.print(" /aftercollecting_time: ");
  SerialUSB.println(mode.aftercollecting_time);
#endif
}
void M_Device::stateConnected(){
  state=CONNECTED;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***CONNECTED");
#endif
}
void M_Device::stateCalibrated(){
  state=READY;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***READY");                 
#endif
}
void M_Device::delayState(){
  previousMillis_start = (int)millis();
  state=WAITING;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***WAITING");
#endif
}
int M_Device::getState(){
  return state;
}
int M_Device::getProtocol(){
  return mode.code;
}
void M_Device::stateError(){
  state=ERROR_STATE;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***ERROR_STATE");
#endif
}
void M_Device::measuringComplete(){
  //sendStoredData(0);
  state = FINISHED;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***FINISHED");
#endif
}

/************************************************
 * Utils
 *************************************************/

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(PIEZO, HIGH);
    delayMicroseconds(tone);
    digitalWrite(PIEZO, LOW);
    delayMicroseconds(tone);
  }
}

void flashLed(int pin, int times, int wait) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);

    if (i + 1 < times) {
      delay(wait);
    }
  }
}
/*
float readFloatFromPayload(ZBRxResponse rx, int i){
 union u_tag {
 byte b[4]; 
 float fval;
 } 
 u;
 u.b[0] = rx.getData(i);
 u.b[1] = rx.getData(i+1);
 u.b[2] = rx.getData(i+2);
 u.b[3] = rx.getData(i+3);
 return u.fval;
 }
 */


/*
//I is the index of the uint8 stored length of the string that comes after it.
 String getStringFromPayload(ZBRxResponse rx, int i){
 int length = rx.getData(i);
 String line= "";
 
 for(int k=i+1; k < (length + i + 1); k++){
 line += (char)rx.getData(k);
 }
 return line;
 }
 */

