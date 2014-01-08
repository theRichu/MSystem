// States
#define INITIAL             0
#define CONNECTED           1
#define TIME_CALIBRATED     2
#define READY               3
#define MEASURING           4
#define WAITING             5
#define FINISHED            6

//pins
#define LASER_SENSOR        15 //(PA15)
#define TEST_SWITCH         17 //(PB4)

#define LED_RED             18  //PB12  26?
#define LED_GREEN           19  //PB13  27?
#define LED_YELLOW          20  //PB14  28?
#define PIEZO               21  //PB15  29?

#define BLINK_DELAY       500


int state = INITIAL;  // state variable controls state machine

void  setup()
{
SerialUSB.begin();    // For Debug

pinMode(BOARD_BUTTON_PIN, INPUT_PULLDOWN);
Serial2.begin(9600);     // Xbee Pro

pinMode(LASER_SENSOR,INPUT);
pinMode(TEST_SWITCH,INPUT);
pinMode(LED_RED,OUTPUT);
pinMode(LED_GREEN,OUTPUT);
pinMode(LED_YELLOW,OUTPUT);
pinMode(PIEZO,OUTPUT);

attachInterrupt(LASER_SENSOR,laserCut, RISING);
attachInterrupt(LASER_SENSOR,laserRestoration, FALLING);
Serial2.attachInterrupt(serialInterrupt);
}

void  loop()
{
int buttonState = digitalRead(BOARD_BUTTON_PIN);
static int lastState = LOW;

  if(buttonState==HIGH && buttonState!=lastState){ //if button is pushed, means 3.3V(HIGH) is connected to BOARD_BUTTON_PIN
    if(state==FINISHED){state=INITIAL;}else{state++;}
    lastState = buttonState;
  }

//if(state != INITIAL && digitalRead(LASER_SENSOR)==LOW) // If we're in a running state and the system power switch is turned off, we should stop
//  {
//  delay(50); // debounce
// state = STOPPING;  
//  }  

  
switch(state)
  {
  case INITIAL:
    CheckForStart();
    break;
  case CONNECTED:
    DoConnected();
  case TIME_CALIBRATED:
    DoCalib();
    break;  
  case READY:
    DoReady();
    break;  
  case MEASURING:
    DoMeasure();
    break;  
  case WAITING:
    DoWait();
    break;
  case FINISHED:

    break;
  default:
    SerialUSB.print("Unknown state ");
    SerialUSB.println(state);
   break; 
  }
}

void serialInterrupt(byte buffer){
  //Xbee Process
  //JSON Parsing
  
  SerialUSB.print((char)buffer);
}

void laserCut(void){
  SerialUSB.println("Laser Cut");
}

void laserRestoration(void){
  SerialUSB.println("Laser Restoration");
}

void toggleLED_R()
{
  togglePin(LED_RED);
}

void toggleLED_Y()
{
  togglePin(LED_YELLOW);
}

void toggleLED_G()
{
  togglePin(LED_GREEN);
}

void     CheckForStart()
{ 
  static long previousMillis = 0;
//Confirm attached devices
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_YELLOW,HIGH);
  if ((int)millis() - previousMillis > BLINK_DELAY) {
    previousMillis = millis();
    toggleLED_G();
  }

// Xbee Wait
  //If Connected

//
}

void DoConnected()
{
  static long previousMillis = 0;
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_YELLOW,LOW);

  if ((int)millis() - previousMillis > BLINK_DELAY) {
    previousMillis = millis();
    toggleLED_G();
  }
}

void DoCalib()
{
  static long previousMillis = 0;
  digitalWrite(LED_RED,HIGH);
  digitalWrite(LED_GREEN,LOW);

  if ((int)millis() - previousMillis > BLINK_DELAY) {
    previousMillis = millis();
    toggleLED_Y();
  }  
}
void DoReady()
{
  digitalWrite(LED_RED,HIGH);
  digitalWrite(LED_YELLOW,HIGH);
  digitalWrite(LED_GREEN,HIGH);
}

void DoMeasure()
{
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_YELLOW,LOW);
  digitalWrite(LED_GREEN,HIGH);  
}

void DoWait()
{
  digitalWrite(LED_RED,LOW);
  digitalWrite(LED_YELLOW,HIGH);
  digitalWrite(LED_GREEN,LOW);  
}

void DoStop()
{
  SerialUSB.println("Stopping");
  static long previousMillis = 0;
  digitalWrite(LED_YELLOW,LOW);
  digitalWrite(LED_GREEN,LOW);

  if ((int)millis() - previousMillis > BLINK_DELAY) {
    previousMillis = millis();
    toggleLED_R();
  }  
}
