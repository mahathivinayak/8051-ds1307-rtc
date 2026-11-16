/*
 * ds1307.c - driver for the Maxim DS1307 real-time clock.
 *
 * The DS1307 keeps time in 8 registers (0x00-0x07) holding BCD values:
 *   0x00 seconds (bit 7 = CH, clock halt)   0x04 date
 *   0x01 minutes                            0x05 month
 *   0x02 hours (bit 6 = 12/24-hour mode)    0x06 year
 *   0x03 day of week                        0x07 control (square-wave output)
 *
 * Reads set the register pointer to 0x00, issue a repeated START and read
 * all 7 time registers in one burst, so the values are consistent with each
 * other (the chip latches them at the START condition).
 */
#include "ds1307.h"
#include "i2c.h"

#define DS1307_ADDR_W  0xD0       /* 7-bit address 0x68 + write */
#define DS1307_ADDR_R  0xD1       /* 7-bit address 0x68 + read  */

#define SEC_CH         0x80       /* clock-halt bit in the seconds register */
#define HOUR_12H       0x40       /* 12-hour mode bit */
#define HOUR_PM        0x20       /* PM bit (12-hour mode only) */

unsigned char bcd_to_bin(unsigned char bcd)
{
    return (unsigned char)((bcd >> 4) * 10 + (bcd & 0x0F));
}

unsigned char bin_to_bcd(unsigned char bin)
{
    return (unsigned char)(((bin / 10) << 4) | (bin % 10));
}

unsigned char ds1307_read(rtc_time_t *t)
{
    unsigned char r[7];
    unsigned char i, h;

    i2c_start();
    if (!i2c_write(DS1307_ADDR_W)) {
        i2c_stop();
        return DS1307_NO_ACK;
    }
    i2c_write(0x00);                       /* register pointer -> seconds */

    i2c_start();                           /* repeated START, switch to reading */
    i2c_write(DS1307_ADDR_R);
    for (i = 0; i < 7; i++)
        r[i] = i2c_read(i < 6);            /* ACK every byte except the last */
    i2c_stop();

    t->sec = bcd_to_bin(r[0] & 0x7F);
    t->min = bcd_to_bin(r[1] & 0x7F);

    h = r[2];
    if (h & HOUR_12H) {                    /* chip was set in 12-hour mode elsewhere */
        unsigned char h12 = bcd_to_bin(h & 0x1F);
        t->hour = (unsigned char)((h12 % 12) + ((h & HOUR_PM) ? 12 : 0));
    } else {
        t->hour = bcd_to_bin(h & 0x3F);
    }

    t->day   = r[3] & 0x07;
    t->date  = bcd_to_bin(r[4] & 0x3F);
    t->month = bcd_to_bin(r[5] & 0x1F);
    t->year  = bcd_to_bin(r[6]);

    return (r[0] & SEC_CH) ? DS1307_HALTED : DS1307_OK;
}

unsigned char ds1307_set(const rtc_time_t *t)
{
    i2c_start();
    if (!i2c_write(DS1307_ADDR_W)) {
        i2c_stop();
        return DS1307_NO_ACK;
    }
    i2c_write(0x00);                                  /* start at the seconds register */
    i2c_write(bin_to_bcd(t->sec) & 0x7F);             /* CH = 0: oscillator runs */
    i2c_write(bin_to_bcd(t->min));
    i2c_write(bin_to_bcd(t->hour) & 0x3F);            /* bit 6 = 0: 24-hour mode */
    i2c_write(t->day);
    i2c_write(bin_to_bcd(t->date));
    i2c_write(bin_to_bcd(t->month));
    i2c_write(bin_to_bcd(t->year));
    i2c_write(0x00);                                  /* control: square-wave output off */
    i2c_stop();
    return DS1307_OK;
}
