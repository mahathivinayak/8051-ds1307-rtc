/*
 * delay.c - accurate millisecond delay using Timer0 in mode 1 (16-bit).
 *
 * The 8051 timer counts once per machine cycle = 12 oscillator periods.
 * At 11.0592 MHz that is 921 600 counts/s, so 1 ms = 921 counts and the
 * timer is preloaded with 65536 - 921 so that it overflows after 1 ms.
 */
#include <reg51.h>
#include "delay.h"

#define T0_COUNTS_PER_MS  (FOSC / 12UL / 1000UL)
#define T0_RELOAD         (65536UL - T0_COUNTS_PER_MS)

void delay_ms(unsigned int ms)
{
    TMOD = (TMOD & 0xF0) | 0x01;          /* Timer0 mode 1, leave Timer1 settings alone */

    while (ms--) {
        TH0 = (unsigned char)(T0_RELOAD >> 8);
        TL0 = (unsigned char)(T0_RELOAD & 0xFF);
        TF0 = 0;
        TR0 = 1;
        while (!TF0)                      /* wait for overflow */
            ;
        TR0 = 0;
    }
}
