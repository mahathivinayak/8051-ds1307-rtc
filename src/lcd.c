/*
 * lcd.c - HD44780-compatible 16x2 LCD in 8-bit mode.
 *
 * Data bus on port 1, RS and EN on port 3, R/W tied to GND (write-only),
 * so fixed delays are used instead of polling the busy flag.
 */
#include <reg51.h>
#include <intrins.h>
#include "lcd.h"
#include "delay.h"

#define LCD_DATA  P1              /* D0-D7 - change these three lines for your board */
sbit LCD_RS = P3^5;               /* 0 = command, 1 = data */
sbit LCD_EN = P3^6;               /* latches on the falling edge */

/* HD44780 commands */
#define CMD_CLEAR        0x01
#define CMD_ENTRY_INC    0x06     /* cursor moves right, no display shift */
#define CMD_DISPLAY_ON   0x0C     /* display on, cursor off, blink off */
#define CMD_8BIT_2LINE   0x38     /* 8-bit bus, 2 lines, 5x8 font */
#define CMD_DDRAM_ROW0   0x80
#define CMD_DDRAM_ROW1   0xC0

static void pulse_enable(void)
{
    LCD_EN = 1;
    _nop_(); _nop_();             /* EN high for > 450 ns */
    LCD_EN = 0;
}

void lcd_cmd(unsigned char c)
{
    LCD_RS = 0;
    LCD_DATA = c;
    pulse_enable();
    delay_ms(2);                  /* clear/home need 1.52 ms, others 37 us */
}

void lcd_putc(char c)
{
    LCD_RS = 1;
    LCD_DATA = (unsigned char)c;
    pulse_enable();
    delay_ms(1);
}

void lcd_init(void)
{
    LCD_EN = 0;
    delay_ms(20);                 /* > 15 ms after power-up */
    lcd_cmd(CMD_8BIT_2LINE);      /* function set, sent twice per the datasheet init sequence */
    delay_ms(5);
    lcd_cmd(CMD_8BIT_2LINE);
    lcd_cmd(CMD_DISPLAY_ON);
    lcd_cmd(CMD_ENTRY_INC);
    lcd_cmd(CMD_CLEAR);
}

void lcd_clear(void)
{
    lcd_cmd(CMD_CLEAR);
}

void lcd_goto(unsigned char row, unsigned char col)
{
    lcd_cmd((unsigned char)((row ? CMD_DDRAM_ROW1 : CMD_DDRAM_ROW0) + (col & 0x0F)));
}

void lcd_puts(const char *s)
{
    while (*s)
        lcd_putc(*s++);
}

void lcd_put2(unsigned char v)
{
    lcd_putc((char)('0' + (v / 10) % 10));
    lcd_putc((char)('0' + v % 10));
}
