#include "m_utils.h"

/**********************************************************
 * M_Device Class
 ************************************************************/
long previousMillis_start = 0;

void M_Device::setProtocol(int _p, int _r, int _d, int _a)
{
    mode.code = _p;
    mode.repeat_num = _r;
    mode.delay_time = _d;
    mode.aftercollecting_time = _a;
}

void M_Device::setRole(int _s, int _r)
{
    for (int i = 0; i < SENSOR_NUM; i++)
    {
        if (sensors[i] == _s)
        {
            sensor_roles[i] = _r;
        }
    }
}

void M_Device::stateReset()
{
#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("***INITIALIZED");
#endif
    m_times = 0;
    state = INITIAL;
}

void M_Device::stateEnd()
{
#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("***DRILL END");
#endif
    m_times = 0;
    state = END;
#ifdef VER2
    present(led_black);
#endif
}

void M_Device::stateMeasure()
{
    state = MEASURING;

#ifdef VER2
    present(led_g[getLEDDirection()]);
#endif

#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("***MEASURING");
    SerialUSB.print(" /protocol: ");
    switch (mode.code)
    {
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

void M_Device::stateAftercollect()
{
    previousMillis_start = (int)millis();
    state = AFTERCOLLECTING;

#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("*** AFTERCOLLECTING");
#endif
}

void M_Device::stateConnected()
{
    state = CONNECTED;

#ifdef VER2
    present(led_g[getLEDDirection()]);
#endif

#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("*** CONNECTED");
#endif
}

void M_Device::stateCalibrated()
{
    state = READY;
#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("*** CALIBRATED");
#endif
}

void M_Device::statePause()
{
    state = PAUSE;
#ifdef VER2
    for (int i = 0; i < SENSOR_NUM; i++)
    {
        if (sensors[i] == S_LASER)
        {
            if (sensor_roles[i] == ROLE_START)
            {
                present(led_yg[getLEDDirection()]);
                break;
            }
            else if (sensor_roles[i] == ROLE_END)
            {
                present(led_ry[getLEDDirection()]);
                break;
            }
            else
            {
                present(led_y[getLEDDirection()]);
            }
        }
    }
#endif

#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("*** PAUSE");
#endif
}

void M_Device::stateError()
{
    state = ERROR_STATE;
#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("*** ERROR_STATE");
#endif
}

void M_Device::stateWait()
{
    previousMillis_start = (int)millis();
    state = WAITING;
#ifdef VER2
    present(led_y[getLEDDirection()]);
#endif

#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("*** WAITING");
#endif
}

void M_Device::stateFinish()
{
    //sendStoredData(0);
    state = FINISHED;
#ifdef DEB
    SerialUSB.print(millis());
    SerialUSB.println("*** MEASURING FINISHED");
#endif
}

int M_Device::getState()
{
    return state;
}

int M_Device::getProtocol()
{
    return mode.code;
}

#ifdef VER2
int M_Device::getLEDDirection()
{
    return led_direction;
}
void M_Device::setLEDDirection(int _d)
{
    led_direction = _d % LED_EACH_NUM;

}

unsigned long endTime;

byte led_black[3 * (kNumLEDs)] = {0};

byte led_r[][3 * (kNumLEDs)] = {{
        255, 0, 0, 255, 0, 0, 255, 0, 0,
        0, 0, 0,  0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        255, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0,  0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 255, 0, 0, 0, 0, 0,
        0, 0, 0,  0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 255, 0, 0,
        0, 0, 0,  0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

byte led_y[][3 * (kNumLEDs)] = {{
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        255, 255, 0, 255, 255, 0, 255, 255, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 255, 255, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 255, 255, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        255, 255, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

byte led_g[][3 * (kNumLEDs)] = {{
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 255, 0, 0, 255, 0, 0, 255, 0
    }, {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 255, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 255, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 255, 0
    }
};

byte led_ry[][3 * (kNumLEDs)] =  {{
        255, 0, 0, 255, 0, 0, 255, 0, 0,
        255, 255, 0, 255, 255, 0, 255, 255, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        255, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 255, 255, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 255, 0, 0, 0, 0, 0,
        0, 0, 0, 255, 255, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 255, 0, 0,
        255, 255, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};


byte led_rg[][3 * (kNumLEDs)] =  {{
        255, 0, 0, 255, 0, 0, 255, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 255, 0, 0, 255, 0, 0, 255, 0
    }, {
        255, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 255, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 255, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 255, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 255, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 255, 0
    }
};


byte led_yg[][3 * (kNumLEDs)] =  {{
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        255, 255, 0, 255, 255, 0, 255, 255, 0,
        0, 255, 0, 0, 255, 0, 0, 255, 0
    }, {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 255, 255, 0,
        0, 255, 0, 0, 0, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 255, 255, 0, 0, 0, 0,
        0, 0, 0, 0, 255, 0, 0, 0, 0
    }, {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        255, 255, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 255, 0
    }
};


byte led_all[][3 * (kNumLEDs)] =  {{
        255, 0, 0, 255, 0, 0, 255, 0, 0,
        255, 255, 0, 255, 255, 0, 255, 255, 0,
        0, 255, 0, 0, 255, 0, 0, 255, 0
    },
    {
        255, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 255, 255, 0,
        0, 255, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0, 0, 0, 255, 0, 0, 0, 0, 0,
        0, 0, 0, 255, 255, 0, 0, 0, 0,
        0, 0, 0, 0, 255, 0, 0, 0, 0
    },
    {
        0, 0, 0, 0, 0, 0, 255, 0, 0,
        255, 255, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 255, 0
    }
};



void present(byte *rgb /* = NULL */)
{
    //if (rgb == NULL) rgb = getWriteBuffer();

    // Be sure we leave 50+ microseconds for data latch
    while ((micros() - endTime) < 50L);

    volatile uint32 *setResetRegister = &(PIN_MAP[LED_PIN].gpio_device->regs->BSRR);
    uint32 highValue = (1 << PIN_MAP[LED_PIN].gpio_bit);
    uint32 lowValue = (1 << PIN_MAP[LED_PIN].gpio_bit + 16);
    byte *end = rgb + 3 * kNumLEDs;
    byte r, g, b, mask;

    // Turn off interrupts until we're done with our loop
    noInterrupts();

    while (rgb < end)
    {
        byte r = *rgb++;
        byte g = *rgb++;
        byte b = *rgb++;

        // Our frame buffers are RGB, but WS2812s want colors in GRB order, so we
        // do that reording here.

        // Emit green
        for (mask = 0x80; mask; mask >>= 1)
        {
            *setResetRegister = highValue;
            if (g & mask)
            {
                spinDelay(kSpinCountOneBitHigh);
                *setResetRegister = lowValue;
                spinDelay(kSpinCountOneBitLow);
            }
            else
            {
                spinDelay(kSpinCountZeroBitHigh);
                *setResetRegister = lowValue;
                spinDelay(kSpinCountZeroBitLow);
            }
        }
        // Emit red
        for (mask = 0x80; mask; mask >>= 1)
        {
            *setResetRegister = highValue;
            if (r & mask)
            {
                spinDelay(kSpinCountOneBitHigh);
                *setResetRegister = lowValue;
                spinDelay(kSpinCountOneBitLow);
            }
            else
            {
                spinDelay(kSpinCountZeroBitHigh);
                *setResetRegister = lowValue;
                spinDelay(kSpinCountZeroBitLow);
            }
        }
        // Emit blue
        for (mask = 0x80; mask; mask >>= 1)
        {
            *setResetRegister = highValue;
            if (b & mask)
            {
                spinDelay(kSpinCountOneBitHigh);
                *setResetRegister = lowValue;
                spinDelay(kSpinCountOneBitLow);
            }
            else
            {
                spinDelay(kSpinCountZeroBitHigh);
                *setResetRegister = lowValue;
                spinDelay(kSpinCountZeroBitLow);
            }
        }

    }

    interrupts();
    endTime = micros();
}

#endif
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

