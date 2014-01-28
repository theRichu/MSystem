#ifndef LEPL_M_DEVICE_H
#define LEPL_M_DEVICE_H

#include "wirish.h"
#include <inttypes.h>

////DEBUG
//#define DEB
#define VER2

////////////////INTERNAL/////////////////////
///States
#define INITIAL            0     //DISCONNECTED
#define CONNECTED          1
#define READY              2
#define MEASURING          3
#define AFTERCOLLECTING    4
#define WAITING            5
#define PAUSE              6
#define FINISHED           7
#define END                8

#define ERROR_STATE        9


/////////////////////COMM//////////////////////////////
#define RECORD                30
#define SIGNAL                10
#define SIGNAL_RESP           1
#define SIGNAL_DELAY_REQ      2
#define SIGNAL_DELAY_FEEDBACK 3
#define SIGNAL_START          9
#define SIGNAL_END            8
#define STATUS_RESP           20


///Protocols
#define  P_UNDEFINED        0x00
#define  P_NORMAL           0x10
#define  P_LAP              0x20

///SENSOR TYPE
#define S_LASER             0x10
#define S_RFID              0x20
#define S_JUMPPAD           0x30
#define S_SHOCK             0x40
#define S_SWITCH            0x50

#define NORMAL_HIGH          0x0
#define NORMAL_LOW           0x1
#define AFTERCOLLECT_HIGH    0x2
#define AFTERCOLLECT_LOW     0x3
#define NORMAL_STREAMING     0x4

///ROLE CODE
#define ROLE_START          0x10
#define ROLE_NORMAL         0x20
#define ROLE_END            0x30



///ZIGBEE CODES

////ORDER_CODE
#define SYNC                0x10
#define DELAY_RESP          0x14
#define GET_STATUS          0x20
#define SET_STATUS          0x22
#define DRILL_READY         0x30
#define DRILL_RESET         0x31
///RESPONSE_CODE
#define RESP                0x12
#define DELAY_REQ           0x18
#define DELAY_FEEDBACK      0x15
#define MEASURE_START       0x41
#define MEASURE_END         0x42
#define MEASURE_ERROR       0x43
#define MEASURE_OK          0x44
#define MEASURE_READY       0x45
#define MEASURE_RESETTED    0x46
#define STATUS              0x21

//pins
#define LASER_SENSOR        8   //PCB PRINT 8
#define PIEZO               13  // PCB PRINT 13

#ifdef VER2
#define LED_PIN             11
#else
#define LED_RED             10  // PCB PRINT 10
#define LED_YELLOW          11  // PCB PRINT 11
#define LED_BLUE            12  // PCB PRINT 12
#endif

#define BLINK_DELAY         500
#define LED_CYCLE_DELAY     100
#define SENSOR_NUM          1
#define MAX_BUFFER_NUM      7

#define T_DELAY             500
#define SOUND_LENGTH        50

#ifndef NULL
#define NULL 0
#endif




#ifdef VER2
//////////////////////LED/////////////////////////////
#define kSpinCountZeroBitHigh 4   // these give 330ns high and 800ns low
#define kSpinCountZeroBitLow  9
#define kSpinCountOneBitHigh  9  // these give 750 high and 360 low
#define kSpinCountOneBitLow   4

#define LED_EACH_NUM 3
#define kNumLEDs ((LED_EACH_NUM)*3)


static inline void spinDelay(uint32) __attribute__((always_inline, unused));
static inline void spinDelay(uint32 count)
{
    asm volatile(
        "L_%=_loop:\n\t"
        "subs %0, #1\n\t"
        "bne  L_%=_loop\n"
        : "+r" (count) :
    );
}
// WS2812 chip - NeoPixel, etc.
#endif


struct Protocol
{
    int code;
    int repeat_num;
    int delay_time;
    int aftercollecting_time;
};

class M_Device
{
private:
    int state;// = INITIAL;  // state variable controls state machine

#ifdef VER2
    int led_direction;
#endif

public:
    int sensors[SENSOR_NUM];// = {S_LASER};
    int sensor_roles[SENSOR_NUM];
    int m_times;
    // Node* Popped;
    struct Protocol mode;// = 0;
public:

#ifdef VER2
    int getLEDDirection();
    void setLEDDirection(int _d);
#endif

    int getState();
    void stateAftercollect();
    void stateFinish();
    void stateError();
    void stateCalibrated();
    void stateConnected();
    void stateMeasure();
    void stateReset();
    void stateWait();
    void statePause();
    void stateEnd();

    void setProtocol(int _p, int _r, int _d, int _a);
    void setRole(int _s, int _r);
    int getProtocol();

    //inline void sendStoredData(int numbers);

    M_Device()
    {
        m_times = 0;
        state = INITIAL;
        mode.code = P_UNDEFINED;
        mode.repeat_num = 0;
        mode.delay_time = 0;
        mode.aftercollecting_time = 2;
        sensors[0] = S_LASER;
        sensor_roles[0] = ROLE_NORMAL;

#ifdef VER2
        led_direction = 0;
#endif

    }

};


/****************************************************************
 * Utils
 *****************************************************************/
extern long previousMillis_start;


inline void addByteToPayload(uint8_t *payload_array, byte value);
inline void addTimestampToPayload(uint8_t *payload_array, unsigned long value);
inline void addFloatToPayload(uint8_t *payload_array, float value);


#ifdef VER2

void present(byte *rgb /* = NULL */);

extern byte led_black[3 * (kNumLEDs)];
extern byte led_r[][3 * (kNumLEDs)];
extern byte led_y[][3 * (kNumLEDs)];
extern byte led_g[][3 * (kNumLEDs)];
extern byte led_ry[][3 * (kNumLEDs)];
extern byte led_rg[][3 * (kNumLEDs)];
extern byte led_yg[][3 * (kNumLEDs)] ;
extern byte led_all[][3 * (kNumLEDs)];



inline void led_all_blink()
{
    static int a = 0;
    present(led_all[a++]);
    if (a == 4) a = 0;
}
inline void led_r_blink()
{
    static int r = 0;
    present(led_r[r++]);
    if (r == LED_EACH_NUM + 1) r = 0;
}
inline void led_g_blink()
{
    static int g = 0;
    present(led_g[g++]);
    if (g == LED_EACH_NUM + 1) g = 0;
}
inline void led_y_blink()
{
    static int y = 0;
    present(led_y[y++]);
    if (y == LED_EACH_NUM + 1) y = 0;
}
inline void led_ry_blink()
{
    static int ry = 0;
    present(led_ry[ry++]);
    if (ry == LED_EACH_NUM + 1) ry = 0;
}
inline void led_rg_blink()
{
    static int rg = 0;
    present(led_rg[rg++]);
    if (rg == LED_EACH_NUM + 1) rg = 0;
}
inline void led_yg_blink()
{
    static int yg = 0;
    present(led_yg[yg++]);
    if (yg == LED_EACH_NUM + 1) yg = 0;
}




#endif

#endif
