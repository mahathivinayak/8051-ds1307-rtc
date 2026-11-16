/* i2c.h - bit-banged I2C master for the 8051 (standard mode, < 100 kHz) */
#ifndef I2C_H
#define I2C_H

void          i2c_init(void);
void          i2c_start(void);              /* also used as a repeated start */
void          i2c_stop(void);
bit           i2c_write(unsigned char b);   /* returns 1 if the slave ACKed */
unsigned char i2c_read(bit ack);            /* ack = 1: more bytes to follow, 0: last byte (NACK) */

#endif
