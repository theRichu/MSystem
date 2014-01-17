#ifndef LEPL_M_DEVICE_H
#define LEPL_M_DEVICE_H

#include "wirish.h"
#include <inttypes.h>

////DEBUG
//#define DEB

////////////////INTERNAL/////////////////////
///States
#define INITIAL                           0     //DISCONNECTED
#define CONNECTED                 1
#define READY                           2
#define MEASURING                3
#define AFTERCOLLECTING   4
#define WAITING                       5
#define FINISHED                     6
#define ERROR_STATE           8

/////////////////////COMM//////////////////////////////
///Protocols
#define  P_UNDEFINED          0x00
#define  P_NORMAL                 0x10
#define   P_LAP                         0x20

///SENSOR TYPE
#define S_LASER                  0x10
#define S_RFID                      0x20
#define S_JUMPPAD            0x30
#define S_SHOCK                 0x40
#define S_SWITCH               0x50

#define NORMAL_HIGH                  0x0
#define NORMAL_LOW                    0x1
#define AFTERCOLLECT_HIGH     0x2
#define AFTERCOLLECT_LOW      0x3
#define NORMAL_STREAMING    0x4

///ROLE CODE
#define ROLE_START                0x10
#define ROLE_NORMAL           0x20
#define ROLE_END                    0x30



///ZIGBEE CODES

////ORDER_CODE
#define SYNC                                0x10
#define DELAY_RESP                  0x14
#define GET_STATUS                  0x20
#define SET_STATUS                   0x22
#define DRILL_READY                0x30
#define DRILL_RESET                  0x31
///RESPONSE_CODE
#define RESP                                   0x12
#define DELAY_REQ                      0x13
#define DELAY_FEEDBACK        0x15
#define MEASURE_START           0x41
#define MEASURE_END                0x42
#define MEASURE_ERROR         0x43
#define MEASURE_OK                  0x44
#define MEASURE_READY         0x45
#define MEASURE_RESETTED 0x46
#define STATUS                              0x21

//pins
#define LASER_SENSOR                8   //PCB PRINT 8
#define LED_RED                            10  // PCB PRINT 10
#define LED_YELLOW                    11  // PCB PRINT 11
#define LED_BLUE                          12  // PCB PRINT 12
#define PIEZO                                   13  // PCB PRINT 13
#define LED_ALL_RED                   17  // PCB PRINT 15
#define LED_ALL_YELLOW           18  // PCB PRINT 16
#define LED_ALL_BLUE                 19  // PCB PRINT 17
//#define LED_XBEE_STATUS       17  // PCB PRINT 15
//#define LED_XBEE_ERROR        18  // PCB PRINT 16

#define BLINK_DELAY       500
#define SENSOR_NUM        1
#define DEVICE_ID         2
#define MAX_BUFFER_NUM    32

#define T_DELAY                         500
#define SOUND_LENGTH          50

//TODO : Error
//TODO : Rules

#ifndef NULL
#define NULL 0
#endif

/******************************************************************
 * Linked Queue
 ******************************************************************/
typedef struct tagNode
{
  int Data;
  unsigned long Timestamp;
  struct tagNode* NextNode;
}Node;

typedef struct tagLinkedQueue
{
  Node* Front;
  Node* Rear;
  int Count;
}LinkedQueue;

void LQ_CreateQueue(LinkedQueue** Queue);
void LQ_DestoryQueue(LinkedQueue* Queue);
Node* LQ_CreateNode(int NewData, unsigned long _timestamp);
void LQ_DestoryNode(Node* _Node);
void LQ_Enqueue(LinkedQueue* Queue, Node* NewNode);
Node* LQ_Dequeue(LinkedQueue* Queue);
int LQ_IsEmpty(LinkedQueue* Queue);



struct Protocol{
  int code;
  int repeat_num;
  int delay_time;
  int aftercollecting_time;
};
/*
struct CalibTimedata{
  unsigned long sync;
  unsigned long delay_resp;
};*/

class M_Device{
private:
  int device_id;
  int state;// = INITIAL;  // state variable controls state machine
  int sensors[SENSOR_NUM];// = {S_LASER};
  int sensor_rules[SENSOR_NUM];

public:
  int m_times;
  // int collecting;
  // int laser_state;
  Node* Popped;
  LinkedQueue* Queue;
  struct Protocol mode;// = 0;
public:
  int getState();
  void stateAftercollecting();
  void measuringComplete();
  void stateError();
  void stateCalibrated();
  void stateConnected();
  void stateMeasuring();
  void stateReset();
  void setProtocol(int _p, int _r, int _d, int _a);
  int getProtocol();
  void delayState();
  void sendStoredData(int numbers);

  M_Device(){
    m_times=0;
    state=INITIAL;
    mode.code=P_UNDEFINED;
    mode.repeat_num=0;
    mode.delay_time=0;
    mode.aftercollecting_time=2;
    device_id=DEVICE_ID;
    sensors[0] = S_LASER;
  }
  /*
  M_Device(int d_id, int* _sensors){
   state=INITIAL;
   mode.code=P_UNDEFINED;
   device_id=d_id;
   sensors[0] = S_LASER;
   }*/
};


/****************************************************************
 * Utils
 *****************************************************************/
extern long previousMillis_start;
//struct CalibTimedata d;
void playTone(int tone, int duration);
void flashLed(int pin, int times, int wait);
void addByteToPayload(uint8_t* payload_array, byte value);
void addTimestampToPayload(uint8_t* payload_array, unsigned long value);
void addFloatToPayload(uint8_t* payload_array, float value);

#endif


