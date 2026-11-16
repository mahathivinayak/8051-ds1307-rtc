/* lcd.h - HD44780 16x2 character LCD, 8-bit interface */
#ifndef LCD_H
#define LCD_H

void lcd_init(void);
void lcd_cmd(unsigned char c);
void lcd_putc(char c);
void lcd_puts(const char *s);
void lcd_put2(unsigned char v);           /* two digits with a leading zero */
void lcd_goto(unsigned char row, unsigned char col);
void lcd_clear(void);

#endif
