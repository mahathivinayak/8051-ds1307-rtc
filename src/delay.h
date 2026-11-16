/* delay.h - millisecond delay using Timer0 */
#ifndef DELAY_H
#define DELAY_H

#define FOSC 11059200UL           /* crystal frequency in Hz - change if your board differs */

void delay_ms(unsigned int ms);

#endif
