#include <MapleFreeRTOS.h>
#include "m_utils.h"
#include <XBee.h>


/*******************************************************************************
 * Global
 *******************************************************************************/
//     Class
M_Device m;
//
uint8_t payload[MAX_BUFFER_NUM] = {0};
uint8_t payload_short[5] = {0};
uint8_t payload_broadcast[1] = {0};
uint8_t* receivedData;

uint8_t payloadPointer = 0;


long previousMillis = 0;
long soundMillis=0;
int old_m_times=0;

//uint8_t* p_payload = payload;
//uint8_t* p_payload_short = payload_short;
uint8_t option = 0;

volatile int sound=0;
volatile unsigned long sens_time=0;
volatile int sens_state=0;


//                XBee
XBee xbee = XBee();

XBeeResponse response = XBeeResponse();
Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();

Tx16Request tx_short = Tx16Request(0x0000, payload_short, sizeof(payload_short));
Tx16Request tx_broadcast = Tx16Request(0xFFFF, payload_broadcast, sizeof(payload_broadcast));
Tx16Request tx = Tx16Request(0x0000, payload, sizeof(payload));
TxStatusResponse txStatus = TxStatusResponse();

void  setup()
{
#ifdef DEB
  SerialUSB.begin();    // For Debug
  pinMode(BOARD_BUTTON_PIN,INPUT_PULLDOWN);
  //attachInterrupt(BOARD_BUTTON_PIN,stateChange, FALLING);
#endif
  xbee.begin(9600);

  LQ_CreateQueue(&m.Queue); // Records

  disableDebugPorts();  // for use  11, 12, 13, 17, 18 must be used with this option
  /// input
  pinMode(LASER_SENSOR,INPUT);
  /// output
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_YELLOW,OUTPUT);
  pinMode(LED_BLUE,OUTPUT);
//  pinMode(LED_ALL_RED,OUTPUT);
//  pinMode(LED_ALL_YELLOW,OUTPUT);
//  pinMode(LED_ALL_BLUE,OUTPUT);
  pinMode(PIEZO,OUTPUT);
  
  
  digitalWrite(PIEZO,LOW);
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_YELLOW,LOW);
  digitalWrite(LED_BLUE,LOW);

  attachInterrupt(LASER_SENSOR,laserCut, CHANGE);

  xTaskCreate( vCommTask, ( signed char * ) "Comm", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
  xTaskCreate( vProcessTask, ( signed char * ) "Proc", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
//  xTaskCreate( vPiezoTask, ( signed char * ) "Piezo", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
  vTaskStartScheduler();
}




void laserCut(void){
  sens_time =millis();
  sens_state = digitalRead(LASER_SENSOR);
  if(m.getState()==MEASURING)
  {
    sound=1;
    if(!sens_state)   m.m_times++;
    LQ_Enqueue(m.Queue, LQ_CreateNode(sens_state,sens_time));
  }
  else if(m.getState()==AFTERCOLLECTING)
  {
    sound=2;
    LQ_Enqueue(m.Queue, LQ_CreateNode((sens_state+AFTERCOLLECT_HIGH),sens_time));
     for(int i=0;i<SENSOR_NUM;i++){
    if(m.sensors[i]==S_LASER){
       if (m.sensor_roles[i]==ROLE_END)
       {
            payloadPointer = 0;
         //   tx_short.setOption(DISABLE_ACK_OPTION);
            addByteToPayload(payload_broadcast,MEASURE_END);
            xbee.send(tx_broadcast);
            
            m.stateEnd();
       }
      } 
     }
#ifdef DEB
    SerialUSB.print("-----Laser(");
    SerialUSB.print(sens_state);
    SerialUSB.print(") : ");
    SerialUSB.println(sens_time);
#endif
  }else if(m.getState()==PAUSE)
  {
  
   for(int i=0;i<SENSOR_NUM;i++){
    if(m.sensors[i]==S_LASER){
     if(m.sensor_roles[i]==ROLE_START)
     {
       sound=3;
       if(!sens_state)   m.m_times++;
       LQ_Enqueue(m.Queue, LQ_CreateNode(sens_state,sens_time)); 
       payloadPointer = 0;
       //   tx_short.setOption(DISABLE_ACK_OPTION);
       addByteToPayload(payload_broadcast,MEASURE_START);
       xbee.send(tx_broadcast);
       
       m.stateMeasure();
     }
    } 
   }
  }

#ifdef DEB
    SerialUSB.print("-----Laser(");
    SerialUSB.print(sens_state);
    SerialUSB.print(") : ");
    SerialUSB.print(m.m_times);
    SerialUSB.print(" / ");
    SerialUSB.println(sens_time);
#endif
}
/*
void vPiezoTask( void *pvParameters )
{
  // char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  //  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
  for(;;)  {
    switch(sound){
    case 1:
      for (long i = 0; i < 25000L; i +=  500* 2) {
        digitalWrite(PIEZO, HIGH);
        delayMicroseconds(500);
        digitalWrite(PIEZO, LOW);
        delayMicroseconds(500);
      }
      sound=0;
      break;

    case 2:
      for (long i = 0; i < 20000L; i +=  800* 2) {
        digitalWrite(PIEZO, HIGH);
        delayMicroseconds(800);
        digitalWrite(PIEZO, LOW);
        delayMicroseconds(800);
      }
      sound=0;
      break;
    }
  }
}
*/
void vCommTask( void *pvParameters )
{
  for( ;; )
  { 
    sens_time=millis();   
    xbee.readPacket();    
    if (xbee.getResponse().isAvailable()) {
      // got something      
      if (xbee.getResponse().getApiId() == RX_16_RESPONSE || xbee.getResponse().getApiId() == RX_64_RESPONSE) {
        // got a rx packet        
        if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
          xbee.getResponse().getRx16Response(rx16);
          option = rx16.getOption();
          receivedData = rx16.getData();
        } 
        else {
          xbee.getResponse().getRx64Response(rx64);
          option = rx64.getOption();
          receivedData = rx64.getData();
        }

        //TIME SYNC
        switch(receivedData[0]){
        case SYNC:
          {
            payload_short={0};
            payloadPointer = 0;
         //   tx_short.setOption(DISABLE_ACK_OPTION);
            addByteToPayload(payload_short,RESP);
            xbee.send(tx_short);
            
          //  payload_short={0};
            payloadPointer = 0;
         //   tx_short.setOption(DISABLE_ACK_OPTION);
            addByteToPayload(payload_short,0x18);
            addTimestampToPayload(payload_short,sens_time);
#ifdef DEB
    SerialUSB.print("***t2 :");
    SerialUSB.println(sens_time);
#endif
            delay(T_DELAY);
            xbee.send(tx_short);

            if(m.getState()==INITIAL || m.getState()==ERROR_STATE) m.stateConnected();
            break;
          }
          
          
        case DELAY_RESP:
          {
            payloadPointer = 0;
            payload_short = {0};
            addByteToPayload(payload_short,DELAY_FEEDBACK);
            addTimestampToPayload(payload_short,millis());
            xbee.send(tx_short);
#ifdef DEB
    SerialUSB.print("***t9 :");
    SerialUSB.println(millis());
#endif
            if(m.getState()==CONNECTED)  m.stateCalibrated();
            break;
          }

        case DRILL_READY:
          {
            m.setProtocol(receivedData[1],receivedData[2],receivedData[3],receivedData[4]);   // 1:protocol_code/2:repeat_num/3:delay_time(0.1s)/4:aftercollecting_time(0.1s)
            m.setRole(receivedData[5],receivedData[6]);
            if(m.getState()==READY) m.statePause();
            break;
          }
          
        case MEASURE_START:
            m.stateMeasure();
          break;
        
        case MEASURE_END:
            m.stateEnd();
          break;
       
        case DRILL_RESET:   
          {       
            m.stateReset();
            break;
          }
   
        
        default:
#ifdef DEB
          SerialUSB.print("ERROR_XBEE_CODE");
#endif
          break;
        }

        // TODO check option, rssi bytes    
        //        flashLed(statusLed, 1, 10);

      } 
      else {
        // not something we were expecting
        //m.state = ERROR_STATE;
        //m.stateError();
        //        flashLed(errorLed, 1, 25);    
      }
    } 
    else if (xbee.getResponse().isError()) {
      //m.state = ERROR_STATE;
      //m.stateError();
#ifdef DEB
      SerialUSB.print(millis());
      SerialUSB.print("Error reading packet.  Error code: ");  
      SerialUSB.println(xbee.getResponse().getErrorCode());
#endif
      // or flash error led
    } 

  } 
}



void vProcessTask( void *pvParameters )
{
  for( ;; ){
    if(sound)
     if ((int)millis() - previousMillis > SOUND_LENGTH) {
        togglePin(PIEZO);
        previousMillis =(int)millis();
        sound=0;
     }
    
    switch(m.getState())
    {
    case INITIAL:
      digitalWrite(LED_RED,HIGH);
      digitalWrite(LED_YELLOW,HIGH);
      digitalWrite(LED_BLUE,HIGH);
      break;

    case CONNECTED:
      digitalWrite(LED_RED,LOW);
      digitalWrite(LED_YELLOW,LOW);
      if ((int)millis() - previousMillis > BLINK_DELAY) {
        togglePin(LED_BLUE);
        previousMillis =(int)millis();
      }
      break;

    case READY:
      digitalWrite(LED_RED,LOW);
      digitalWrite(LED_BLUE,LOW);
      if ((int)millis() - previousMillis > BLINK_DELAY/2) {
        togglePin(LED_YELLOW);
        previousMillis =(int)millis();
      }
      break;  

    case MEASURING:
      digitalWrite(LED_RED,LOW);
      digitalWrite(LED_YELLOW,LOW);
      digitalWrite(LED_BLUE,HIGH);  
      if(old_m_times!=m.m_times)
      {
        if(digitalRead(LASER_SENSOR))   
        {  
          m.stateAftercollect();
          old_m_times=m.m_times;
        }
      }
      break;

    case AFTERCOLLECTING:
      digitalWrite(LED_RED,LOW);
      digitalWrite(LED_BLUE,HIGH);

      if ((int)millis() - previousMillis > BLINK_DELAY) {
        togglePin(LED_YELLOW);
        previousMillis =(int)millis();
      }

      if ((int)millis() - previousMillis_start > (m.mode.aftercollecting_time*100)) {
        if(digitalRead(LASER_SENSOR)) {
          if(m.m_times<m.mode.repeat_num){
            if(m.mode.delay_time){        
              m.stateWait();                
            }
            else  { 
             while(m.Queue->Count){
               #ifdef DEB
                SerialUSB.print("Stacked COUNT: ");
                SerialUSB.println(m.Queue->Count);
              #endif
              m.sendStoredData(6);
              }
              m.stateMeasure() ;
            }

          }
          else {
            m.stateFinish();
          }
        }
      }

      break;  

    case WAITING:
      digitalWrite(LED_RED,LOW);
      digitalWrite(LED_YELLOW,HIGH);
      digitalWrite(LED_BLUE,LOW);  

      if ((int)millis() - previousMillis_start > (m.mode.delay_time*100)) {
        if(digitalRead(LASER_SENSOR)) m.stateMeasure();
      }  
      else{
        if ((int)millis() - previousMillis > 100) {
          if(m.Queue->Count){
#ifdef DEB
            SerialUSB.print("Stacked COUNT: ");
            SerialUSB.println(m.Queue->Count);
#endif
            m.sendStoredData(6);
          }      
          previousMillis =(int)millis();
        }    
      }
      break;
      
    case PAUSE:
      for(int i=0;i<SENSOR_NUM;i++){
          if(m.sensors[i]==S_LASER){
           if(m.sensor_roles[i]==ROLE_START)
           {
            digitalWrite(LED_RED,LOW);
            digitalWrite(LED_YELLOW,HIGH);
            digitalWrite(LED_BLUE,HIGH); 
           break; 
           }
           else if (m.sensor_roles[i]==ROLE_END)
           {
            digitalWrite(LED_RED,HIGH);
            digitalWrite(LED_YELLOW,HIGH);
            digitalWrite(LED_BLUE,LOW);  
            break; 
           }
           else
           {
            digitalWrite(LED_RED,LOW);
            digitalWrite(LED_YELLOW,HIGH);
            digitalWrite(LED_BLUE,LOW); 
            break;       
           }
          } 
         }
         
        if ((int)millis() - previousMillis > 100) {
          if(m.Queue->Count){
#ifdef DEB
            SerialUSB.print("Stacked COUNT: ");
            SerialUSB.println(m.Queue->Count);
#endif
            m.sendStoredData(6);
          }      
          previousMillis =(int)millis();
        }    

      break;      
   

    case FINISHED:
      digitalWrite(LED_YELLOW,LOW);
      digitalWrite(LED_BLUE,LOW);
      digitalWrite(LED_RED,LOW);

      if ((int)millis() - previousMillis > 100) {
        if(m.Queue->Count){
#ifdef DEB
          SerialUSB.print("Stacked COUNT: ");
          SerialUSB.println(m.Queue->Count);
#endif
          m.sendStoredData(6);
        }      
        previousMillis =(int)millis();
      }    
      break;
    
    case END:
      digitalWrite(LED_YELLOW,LOW);
      digitalWrite(LED_BLUE,LOW);
      digitalWrite(LED_RED,HIGH);

      if ((int)millis() - previousMillis > 100) {
        if(m.Queue->Count){
#ifdef DEB
          SerialUSB.print("Stacked COUNT: ");
          SerialUSB.println(m.Queue->Count);
#endif
          m.sendStoredData(6);
        }      
        previousMillis =(int)millis();
      }    
      break;

    case ERROR_STATE:
      digitalWrite(LED_YELLOW,LOW);
      digitalWrite(LED_BLUE,LOW);
      digitalWrite(LED_RED,LOW);
      if ((int)millis() - previousMillis > BLINK_DELAY/2) {
        togglePin(LED_RED);
        previousMillis =(int)millis();
      }      
      break;
    default:

      break; 
    }
  }
}



void  loop()
{

}



inline void M_Device::sendStoredData(int numbers){
  //tx.setOption(DISABLE_ACK_OPTION);
  payloadPointer = 0;
  payload={ 0 };

  int num = (Queue->Count<numbers)?Queue->Count:numbers;
  addByteToPayload(payload,MEASURE_OK);
  addByteToPayload(payload,num);
  for(int i=0;i<num;i++)
  {
    Popped = LQ_Dequeue(Queue);
    byte data = Popped->Data + S_LASER+5;
    addByteToPayload(payload,data); //
    addTimestampToPayload(payload,Popped->Timestamp);
    LQ_DestoryNode(Popped);
  }
#ifdef DEB
  SerialUSB.print(payload[0]);
  SerialUSB.println(": data");
#endif

  xbee.send(tx);

#ifdef DEB
  SerialUSB.print(num);
  SerialUSB.println(" SEND");
#endif
}

inline void addByteToPayload(uint8_t* payload_array,  byte value){
  *(payload_array+(payloadPointer++))=value;
}
inline void addTimestampToPayload(uint8_t* payload_array, unsigned long value){
  byte * b = (byte *) &value;
  *(payload_array+(payloadPointer++))=b[0];
  *(payload_array+(payloadPointer++))=b[1];
  *(payload_array+(payloadPointer++))=b[2];
  *(payload_array+(payloadPointer++))=b[3];
}
void addFloatToPayload(uint8_t* payload_array, float value){
  byte * b = (byte *) &value;
  *(payload_array+(payloadPointer++))=b[0];
  *(payload_array+(payloadPointer++))=b[1];
  *(payload_array+(payloadPointer++))=b[2];
  *(payload_array+(payloadPointer++))=b[3];
}
