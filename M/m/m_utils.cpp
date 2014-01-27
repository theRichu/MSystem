#include "m_utils.h"

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

void M_Device::setRole(int _s, int _r){
  for(int i=0;i<SENSOR_NUM;i++){
    if(sensors[i]==_s){
     sensor_roles[i] = _r; 
    }
  }
}

void M_Device::stateReset(){
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***INITIALIZED");
#endif
  m_times=0;
  state=INITIAL;
}

void M_Device::stateEnd(){
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("***DRILL END");
#endif
  m_times=0;
  state=END;
}

void M_Device::stateMeasure(){
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

void M_Device::stateAftercollect(){
  previousMillis_start = (int)millis();
  state=AFTERCOLLECTING;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("*** AFTERCOLLECTING");
#endif
}

void M_Device::stateConnected(){
  state=CONNECTED;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("*** CONNECTED");
#endif
}

void M_Device::stateCalibrated(){
  state=READY;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("*** CALIBRATED");                 
#endif
}

void M_Device::statePause(){
  state=PAUSE;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("*** PAUSE");                 
#endif
}

void M_Device::stateError(){
  state=ERROR_STATE;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("*** ERROR_STATE");
#endif
}

void M_Device::stateWait(){
  previousMillis_start = (int)millis();
  state=WAITING;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("*** WAITING");
#endif
}

void M_Device::stateFinish(){
  //sendStoredData(0);
  state = FINISHED;
#ifdef DEB
  SerialUSB.print(millis());
  SerialUSB.println("*** MEASURING FINISHED");
#endif
}

int M_Device::getState(){
  return state;
}

int M_Device::getProtocol(){
  return mode.code;
}


/************************************************
 * Utils
 *************************************************/

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

