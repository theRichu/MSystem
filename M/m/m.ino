#include "m_utils.h"
#include <MapleFreeRTOS.h>

#include "PriorityQueue.h"
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
uint8_t *receivedData;
uint8_t payloadPointer = 0;

PriorityQueue *txQueue = PQ_Create(2);
PQNode MinNode;

int old_m_times = 0;
uint8_t option = 0;

volatile int sound = 0;
long soundMillis = 0;

volatile unsigned long sens_time = 0;
volatile int sens_state = 0;


//XBee
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
#endif

    xbee.begin(9600);

    disableDebugPorts();  // for use  11, 12, 13, 17, 18 must be used with this option
    /// input
    pinMode(LASER_SENSOR, INPUT);

    attachInterrupt(LASER_SENSOR, laserCut, CHANGE);

    xTaskCreate( vProcessTask, ( signed char *) "Proc", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
    xTaskCreate( vRxTask, ( signed char *) "Rxts", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
    xTaskCreate( vTxTask, ( signed char *) "Txts", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
    vTaskStartScheduler();
}

void laserCut(void)
{
    sens_time = millis();
    sens_state = digitalRead(LASER_SENSOR);
    if (m.getState() == MEASURING)
    {
        soundMillis = (int)millis();
        sound = 1;
        if (!sens_state)   m.m_times++;
        PQ_Enqueue(txQueue, {RECORD, sens_state, sens_time});
    }
    else if (m.getState() == AFTERCOLLECTING)
    {
        soundMillis = (int)millis();
        sound = 2;
        PQ_Enqueue(txQueue, {RECORD, (sens_state + AFTERCOLLECT_HIGH), sens_time});
#ifdef DEB
        SerialUSB.print("-----Laser(");
        SerialUSB.print(sens_state);
        SerialUSB.print(") : ");
        SerialUSB.println(sens_time);
#endif
    }
    else if (m.getState() == PAUSE)
    {
        for (int i = 0; i < SENSOR_NUM; i++)
        {
            if (m.sensors[i] == S_LASER)
            {
                if (m.sensor_roles[i] == ROLE_START)
                {
                    soundMillis = (int)millis();
                    sound = 3;
                    if (!sens_state)   m.m_times++;
                    PQ_Enqueue(txQueue, {SIGNAL_START, MEASURE_START, sens_time});
                    PQ_Enqueue(txQueue, {RECORD, sens_state, sens_time});
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

static void vRxTask( void *pvParameters )
{
    for ( ;; )
    {
        sens_time = millis();
        xbee.readPacket();
        if (xbee.getResponse().isAvailable())
        {
            // got something
            if (xbee.getResponse().getApiId() == RX_16_RESPONSE || xbee.getResponse().getApiId() == RX_64_RESPONSE)
            {
                // got a rx packet
                if (xbee.getResponse().getApiId() == RX_16_RESPONSE)
                {
                    xbee.getResponse().getRx16Response(rx16);
                    option = rx16.getOption();
                    receivedData = rx16.getData();
                }
                else
                {
                    xbee.getResponse().getRx64Response(rx64);
                    option = rx64.getOption();
                    receivedData = rx64.getData();
                }

                //TIME SYNC
                switch (receivedData[0])
                {
                case SYNC:
                {
                    payloadPointer = 0;
                    addByteToPayload(payload_short, RESP);
                    xbee.send(tx_short);

                    PQ_Enqueue(txQueue, {SIGNAL_DELAY_REQ, DELAY_REQ, sens_time});

#ifdef DEB
                    SerialUSB.print("***t2 :");
                    SerialUSB.println(sens_time);
#endif
                    if (m.getState() == INITIAL || m.getState() == ERROR_STATE) m.stateConnected();
                    break;
                }

                case DELAY_RESP:
                {
                    payloadPointer = 0;
                    addByteToPayload(payload_short, DELAY_FEEDBACK);
                    addTimestampToPayload(payload_short, millis());
                    xbee.send(tx_short);
                    //          PQ_Enqueue(txQueue, {SIGNAL_DELAY_FEEDBACK,DELAY_FEEDBACK,sens_time});

#ifdef DEB
                    SerialUSB.print("***t9 :");
                    SerialUSB.println(millis());
#endif
                    if (m.getState() == CONNECTED)  m.stateCalibrated();
                    break;
                }

                case DRILL_READY:
                {
                    m.setProtocol(receivedData[1], receivedData[2], receivedData[3], receivedData[4]); // 1:protocol_code/2:repeat_num/3:delay_time(0.1s)/4:aftercollecting_time(0.1s)
                    m.setRole(receivedData[5], receivedData[6]);
                    if (m.getState() == READY) m.statePause();
                    break;
                }

                case MEASURE_START:
                    m.stateMeasure();
                    break;

                case MEASURE_END:
                    m.stateEnd();
                    break;

                case DRILL_RESET:
                    m.stateReset();
                    break;

                default:

#ifdef DEB
                    SerialUSB.print("ERROR_XBEE_CODE");
#endif
                    break;
                }
                // TODO check option, rssi bytes
                //        flashLed(statusLed, 1, 10);
            }
            else
            {
                // not something we were expecting
                //m.state = ERROR_STATE;
                //m.stateError();
                //        flashLed(errorLed, 1, 25);
            }
        }
        else if (xbee.getResponse().isError())
        {
            //m.state = ERROR_STATE;
            //m.stateError();
#ifdef DEB
            SerialUSB.print(millis());
            SerialUSB.print("Error reading packet.  Error code: ");
            SerialUSB.println(xbee.getResponse().getErrorCode());
#endif
            // or flash error led
        }
        vTaskDelay( 50 );
    }
}


static void vTxTask( void *pvParameters )
{
    int record_num = 0;

    for ( ;; )
    {
        if (txQueue->UsedSize)
        {
            PQ_Dequeue(txQueue, &MinNode);
#ifdef DEB
            SerialUSB.print("Queued Task : ");
            SerialUSB.println(txQueue->UsedSize);
            SerialUSB.print("Work(");
            SerialUSB.print(MinNode.Priority);
            SerialUSB.print(") : ");
            SerialUSB.print(MinNode.Type);
            SerialUSB.print(" / ");
            SerialUSB.println(MinNode.Timestamp);
#endif

            switch (MinNode.Priority)
            {
            case SIGNAL_RESP:
                payload_short = { 0 };
            case SIGNAL_DELAY_REQ:
            case SIGNAL_DELAY_FEEDBACK:
            case SIGNAL:
                //Short
                payloadPointer = 0;
                addByteToPayload(payload_short, MinNode.Type);
                addTimestampToPayload(payload_short, MinNode.Timestamp);
                xbee.send(tx_short);
                break;

            case SIGNAL_START:
            case SIGNAL_END:
                //Broadcast
                payloadPointer = 0;
                //   tx_short.setOption(DISABLE_ACK_OPTION);
                addByteToPayload(payload_broadcast, MinNode.Type);
                xbee.send(tx_broadcast);
                break;

            case RECORD:
                payload = {0};
                payloadPointer = 0;
                addByteToPayload(payload, MEASURE_OK);
                addByteToPayload(payload, 0x01);
                addByteToPayload(payload, MinNode.Type);
                addTimestampToPayload(payload, MinNode.Timestamp);
                xbee.send(tx);
                break;
            }
        }
        vTaskDelay( 50 );
    }
}


static void vProcessTask( void *pvParameters )
{

#ifdef VER2
    pinMode(LED_PIN, OUTPUT);
#else
    /// output
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(PIEZO, OUTPUT);

    digitalWrite(PIEZO, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_BLUE, LOW);
#endif

    long previousMillis = 0;

    for ( ;; )
    {
        ////SOUND
        if (sound)
        {
            digitalWrite(PIEZO, HIGH);
            if ((int)millis() - soundMillis > SOUND_LENGTH)
            {
                sound = 0;
                digitalWrite(PIEZO, LOW);
            }
        }
        ////LED
        switch (m.getState())
        {
        case INITIAL:

#ifdef VER2
            if ((int)millis() - previousMillis > LED_CYCLE_DELAY)
            {
                led_all_blink();
                previousMillis = (int)millis();
            }
#else
            digitalWrite(LED_RED, HIGH);
            digitalWrite(LED_YELLOW, HIGH);
            digitalWrite(LED_BLUE, HIGH);
#endif
            break;

#ifndef VER2
        case CONNECTED:

            digitalWrite(LED_RED, LOW);
            digitalWrite(LED_YELLOW, LOW);
            if ((int)millis() - previousMillis > BLINK_DELAY)
            {
                togglePin(LED_BLUE);
                previousMillis = (int)millis();
            }
            break;
#endif

        case READY:
#ifdef VER2
            if ((int)millis() - previousMillis > LED_CYCLE_DELAY)
            {
                led_y_blink();
                previousMillis = (int)millis();
            }

#else
            digitalWrite(LED_RED, LOW);
            digitalWrite(LED_BLUE, LOW);
            if ((int)millis() - previousMillis > BLINK_DELAY / 2)
            {
                togglePin(LED_YELLOW);
                previousMillis = (int)millis();
            }
#endif
            break;

#ifndef VER2
        case PAUSE:

            for (int i = 0; i < SENSOR_NUM; i++)
            {
                if (m.sensors[i] == S_LASER)
                {
                    if (m.sensor_roles[i] == ROLE_START)
                    {
                        digitalWrite(LED_RED, LOW);
                        digitalWrite(LED_YELLOW, HIGH);
                        digitalWrite(LED_BLUE, HIGH);
                        break;
                    }
                    else if (m.sensor_roles[i] == ROLE_END)
                    {
                        digitalWrite(LED_RED, HIGH);
                        digitalWrite(LED_YELLOW, HIGH);
                        digitalWrite(LED_BLUE, LOW);
                        break;
                    }
                    else
                    {
                        digitalWrite(LED_RED, LOW);
                        digitalWrite(LED_YELLOW, HIGH);
                        digitalWrite(LED_BLUE, LOW);
                    }
                }
            }
            break;
#endif
        case MEASURING:
#ifndef VER2
            digitalWrite(LED_RED, LOW);
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_BLUE, HIGH);
#endif
            if (old_m_times != m.m_times)
            {
                if (digitalRead(LASER_SENSOR))
                {
                    m.stateAftercollect();
                    old_m_times = m.m_times;
                }
            }

            break;

        case AFTERCOLLECTING:
            //NOT DISPLAY
#ifndef VER2
            digitalWrite(LED_RED, LOW);
            digitalWrite(LED_BLUE, HIGH);

            if ((int)millis() - previousMillis > BLINK_DELAY)
            {
                togglePin(LED_YELLOW);
                previousMillis = (int)millis();
            }
#endif
            if ((int)millis() - previousMillis_start > (m.mode.aftercollecting_time * 100))
            {
                if (digitalRead(LASER_SENSOR))
                {
                    if (m.m_times < m.mode.repeat_num)
                    {
                        if (m.mode.delay_time)
                        {
                            m.stateWait();
                        }
                        else
                        {
                            m.stateMeasure() ;
                        }
                    }
                    else
                    {
                        for (int i = 0; i < SENSOR_NUM; i++)
                        {
                            if (m.sensors[i] == S_LASER)
                            {
                                if (m.sensor_roles[i] == ROLE_END)
                                {
                                    PQ_Enqueue(txQueue, {SIGNAL_END, MEASURE_END, sens_time});
                                    m.stateEnd();
                                }
                            }
                            else
                            {
                                m.stateFinish();
                            }
                        }


                    }
                }
            }
            break;

        case WAITING:
#ifndef VER2
            digitalWrite(LED_RED, LOW);
            digitalWrite(LED_YELLOW, HIGH);
            digitalWrite(LED_BLUE, LOW);
#endif
            if ((int)millis() - previousMillis_start > (m.mode.delay_time * 100))
            {
                if (digitalRead(LASER_SENSOR)) m.stateMeasure();
            }
            break;

#ifndef VER2
        case FINISHED:
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_BLUE, LOW);
            digitalWrite(LED_RED, LOW);
            break;

        case END:
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_BLUE, LOW);
            digitalWrite(LED_RED, HIGH);
            break;
#endif
        case ERROR_STATE:
#ifdef VER2
            if ((int)millis() - previousMillis > LED_CYCLE_DELAY)
            {
                led_r_blink();
                previousMillis = (int)millis();
            }
#else
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_BLUE, LOW);
            digitalWrite(LED_RED, LOW);
            if ((int)millis() - previousMillis > BLINK_DELAY / 2)
            {
                togglePin(LED_RED);
                previousMillis = (int)millis();
            }
#endif
            break;
        }
        vTaskDelay( 50 );
    }
}



void  loop()
{

}


inline void addByteToPayload(uint8_t *payload_array,  byte value)
{
    *(payload_array + (payloadPointer++)) = value;
}
inline void addTimestampToPayload(uint8_t *payload_array, unsigned long value)
{
    byte *b = (byte *) &value;
    *(payload_array + (payloadPointer++)) = b[0];
    *(payload_array + (payloadPointer++)) = b[1];
    *(payload_array + (payloadPointer++)) = b[2];
    *(payload_array + (payloadPointer++)) = b[3];
}
void addFloatToPayload(uint8_t *payload_array, float value)
{
    byte *b = (byte *) &value;
    *(payload_array + (payloadPointer++)) = b[0];
    *(payload_array + (payloadPointer++)) = b[1];
    *(payload_array + (payloadPointer++)) = b[2];
    *(payload_array + (payloadPointer++)) = b[3];
}
