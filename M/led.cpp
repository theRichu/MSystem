#define kSpinCountZeroBitHigh 4   // these give 330ns high and 800ns low
#define kSpinCountZeroBitLow  9
#define kSpinCountOneBitHigh  9  // these give 750 high and 360 low
#define kSpinCountOneBitLow   4

#define _pin 11
#define kNumLEDs 9
#define R_NUM 3
#define G_NUM 3
#define B_NUM 3


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




void setup()
{
    // put your setup code here, to run once:
    pinMode(_pin, OUTPUT);
}

unsigned long _endTime;

void present(byte *rgb /* = NULL */)
{
    //if (rgb == NULL) rgb = getWriteBuffer();

    // Be sure we leave 50+ microseconds for data latch
    while ((micros() - _endTime) < 50L);

    volatile uint32 *setResetRegister = &(PIN_MAP[_pin].gpio_device->regs->BSRR);
    uint32 highValue = (1 << PIN_MAP[_pin].gpio_bit);
    uint32 lowValue = (1 << PIN_MAP[_pin].gpio_bit + 16);
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
    _endTime = micros();
}
byte black[3 * (R_NUM + G_NUM + B_NUM)] = {0};

byte red[3] = {255, 0, 0};
byte green[3] = {0, 255, 0};
byte yellow[3] = {255, 255, 0};
byte blue[3] = {0, 0, 255};

byte *led[3 * (R_NUM + G_NUM + B_NUM)] = {red, red, red, yellow, yellow, yellow, green, green, green};

byte led_r[] = {  255, 0, 0, 255, 0, 0, 255, 0, 0,
                  0, 0, 0,  0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0
               };
byte led_r1[] = {  255, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0,  0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0
                };
byte led_r2[] = {  0, 0, 0, 255, 0, 0, 0, 0, 0,
                   0, 0, 0,  0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0
                };
byte led_r3[] = {  0, 0, 0, 0, 0, 0, 255, 0, 0,
                   0, 0, 0,  0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0
                };

byte led_y[] = {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                  255, 255, 0, 255, 255, 0, 255, 255, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0
               };
byte led_y1[] = {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 255, 255, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0
                };
byte led_y2[] = {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 255, 255, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0
                };
byte led_y3[] = {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                   255, 255, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0
                };

byte led_g[] = {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 255, 0, 0, 255, 0, 0, 255, 0
               };
byte led_g1[] = {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 255, 0, 0, 0, 0, 0, 0, 0
                };
byte led_g2[] = {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 255, 0, 0, 0, 0
                };
byte led_g3[] = {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 255, 0
                };

byte led_ry[] =  {  255, 0, 0, 255, 0, 0, 255, 0, 0,
                    255, 255, 0, 255, 255, 0, 255, 255, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                 };
byte led_ry1[] =  {  255, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 255, 255, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0
                  };

byte led_ry2[] =  {  0, 0, 0, 255, 0, 0, 0, 0, 0,
                     0, 0, 0, 255, 255, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0
                  };

byte led_ry3[] =  {  0, 0, 0, 0, 0, 0, 255, 0, 0,
                     255, 255, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0
                  };


byte led_rg[] =  {  255, 0, 0, 255, 0, 0, 255, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 255, 0, 0, 255, 0, 0, 255, 0
                 };
byte led_rg1[] =  {  255, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 255, 0, 0, 0, 0, 0, 0, 0
                  };
byte led_rg2[] =  {  0, 0, 0, 255, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 255, 0, 0, 0, 0
                  };
byte led_rg3[] =  {  0, 0, 0, 0, 0, 0, 255, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 255, 0
                  };


byte led_yg[] =  {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                    255, 255, 0, 255, 255, 0, 255, 255, 0,
                    0, 255, 0, 0, 255, 0, 0, 255, 0
                 };

byte led_yg1[] =  {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 255, 255, 0,
                     0, 255, 0, 0, 0, 0, 0, 0, 0
                  };
byte led_yg2[] =  {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 255, 255, 0, 0, 0, 0,
                     0, 0, 0, 0, 255, 0, 0, 0, 0
                  };
byte led_yg3[] =  {  0, 0, 0, 0, 0, 0, 0, 0, 0,
                     255, 255, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, 0, 255, 0
                  };


byte led_all[] =  {  255, 0, 0, 255, 0, 0, 255, 0, 0,
                     255, 255, 0, 255, 255, 0, 255, 255, 0,
                     0, 255, 0, 0, 255, 0, 0, 255, 0
                  };
byte led_all1[] =  {  255, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 255, 255, 0,
                      0, 255, 0, 0, 0, 0, 0, 0, 0
                   };
byte led_all2[] =  {  0, 0, 0, 255, 0, 0, 0, 0, 0,
                      0, 0, 0, 255, 255, 0, 0, 0, 0,
                      0, 0, 0, 0, 255, 0, 0, 0, 0
                   };
byte led_all3[] =  {  0, 0, 0, 0, 0, 0, 255, 0, 0,
                      255, 255, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 255, 0
                   };
void loop()
{
    present(led_all);
    delay(100);
    present(led_all1);
    delay(100);
    present(led_all2);
    delay(100);
    present(led_all3);
    delay(100);

    present(led_ry);
    delay(100);
    present(led_ry1);
    delay(100);
    present(led_ry2);
    delay(100);
    present(led_ry3);
    delay(100);

    present(led_y);
    delay(100);
    present(led_y1);
    delay(100);
    present(led_y2);
    delay(100);
    present(led_y3);
    delay(100);


    present(led_yg);
    delay(100);
    present(led_yg1);
    delay(100);
    present(led_yg2);
    delay(100);
    present(led_yg3);
    delay(100);

    present(led_g);
    delay(100);
    present(led_g1);
    delay(100);
    present(led_g2);
    delay(100);
    present(led_g3);
    delay(100);

    present(led_rg);
    delay(100);
    present(led_rg1);
    delay(100);
    present(led_rg2);
    delay(100);
    present(led_rg3);
    delay(100);

    present(black);
    delay(1000);
}
