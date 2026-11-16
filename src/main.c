/*
 * main.c - DS1307 real-time clock on an 8051 with a 16x2 LCD.
 *
 *   TIME 14:30:05 MON
 *   DATE 16/11/2026
 *
 * To set the clock: set SET_TIME to 1, fill in the time below, flash once,
 * then set SET_TIME back to 0 and flash again. Otherwise every reset would
 * set the clock back to the same time. The DS1307's backup battery keeps
 * the time while the board is off.
 */
#include <reg51.h>
#include "delay.h"
#include "ds1307.h"
#include "i2c.h"
#include "lcd.h"

#define SET_TIME 0

static const char code day_names[8][4] = { "---", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

static void show_message(const char *line1, const char *line2)
{
    lcd_clear();
    lcd_goto(0, 0);
    lcd_puts(line1);
    lcd_goto(1, 0);
    lcd_puts(line2);
}

static void show_time(const rtc_time_t *t)
{
    lcd_goto(0, 0);
    lcd_puts("TIME ");
    lcd_put2(t->hour);
    lcd_putc(':');
    lcd_put2(t->min);
    lcd_putc(':');
    lcd_put2(t->sec);
    lcd_putc(' ');
    lcd_puts(day_names[(t->day >= 1 && t->day <= 7) ? t->day : 0]);

    lcd_goto(1, 0);
    lcd_puts("DATE ");
    lcd_put2(t->date);
    lcd_putc('/');
    lcd_put2(t->month);
    lcd_puts("/20");
    lcd_put2(t->year);
}

void main(void)
{
    rtc_time_t now;
    unsigned char status;
    unsigned char last_status = 0xFF;
    unsigned char last_sec = 0xFF;

    i2c_init();
    lcd_init();
    show_message("8051 + DS1307", "RTC clock");
    delay_ms(1000);

#if SET_TIME
    now.sec = 0;   now.min = 30;   now.hour = 14;
    now.day = 2;   /* 1 = SUN ... 7 = SAT */
    now.date = 16; now.month = 11; now.year = 26;
    ds1307_set(&now);
#endif

    while (1) {
        status = ds1307_read(&now);

        if (status != last_status) {               /* state changed: redraw the whole screen */
            if (status == DS1307_NO_ACK)
                show_message("RTC not found", "Check SDA/SCL");
            else if (status == DS1307_HALTED)
                show_message("Clock halted", "Set the time");
            else
                lcd_clear();
            last_status = status;
            last_sec = 0xFF;
        }

        if (status == DS1307_OK && now.sec != last_sec) {
            show_time(&now);
            last_sec = now.sec;
        }

        delay_ms(100);                             /* poll 10x per second */
    }
}
