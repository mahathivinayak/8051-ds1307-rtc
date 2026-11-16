/* ds1307.h - driver for the Maxim DS1307 I2C real-time clock */
#ifndef DS1307_H
#define DS1307_H

typedef struct {
    unsigned char sec;            /* 0-59 */
    unsigned char min;            /* 0-59 */
    unsigned char hour;           /* 0-23 */
    unsigned char day;            /* 1-7, 1 = Sunday (this project's convention) */
    unsigned char date;           /* 1-31 */
    unsigned char month;          /* 1-12 */
    unsigned char year;           /* 0-99 -> 2000-2099 */
} rtc_time_t;

#define DS1307_OK       0
#define DS1307_NO_ACK   1         /* chip not responding: wiring, power or pull-ups */
#define DS1307_HALTED   2         /* oscillator stopped (CH bit set): time not running, needs setting */

unsigned char ds1307_read(rtc_time_t *t);
unsigned char ds1307_set(const rtc_time_t *t);   /* also starts the oscillator, 24-hour mode */

unsigned char bcd_to_bin(unsigned char bcd);
unsigned char bin_to_bcd(unsigned char bin);

#endif
