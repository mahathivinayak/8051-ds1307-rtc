/*
 * i2c.c - bit-banged I2C master for the 8051.
 *
 * The 8051 has no I2C peripheral, so SDA and SCL are driven from port pins.
 * 8051 port pins are quasi-bidirectional: writing 1 releases the line (it is
 * pulled high by the pull-up resistor) and writing 0 pulls it low, which is
 * exactly the open-drain behaviour I2C needs. Writing 1 to SDA before
 * sampling it lets the slave drive the line.
 *
 * Each half bit period is a few microseconds at 11.0592 MHz, keeping SCL
 * below the DS1307's 100 kHz limit.
 */
#include <reg51.h>
#include <intrins.h>
#include "i2c.h"

sbit SCL = P2^0;                  /* change these two lines for your board */
sbit SDA = P2^1;

static void half_bit(void)
{
    _nop_(); _nop_(); _nop_(); _nop_();
}

void i2c_init(void)
{
    SDA = 1;                      /* both lines released = bus idle */
    SCL = 1;
}

/* START: SDA falls while SCL is high. */
void i2c_start(void)
{
    SDA = 1;
    SCL = 1;
    half_bit();
    SDA = 0;
    half_bit();
    SCL = 0;
}

/* STOP: SDA rises while SCL is high. */
void i2c_stop(void)
{
    SDA = 0;
    half_bit();
    SCL = 1;
    half_bit();
    SDA = 1;
    half_bit();
}

/* Shift out 8 bits MSB first, then read the slave's ACK bit (0 = ACK). */
bit i2c_write(unsigned char b)
{
    unsigned char i;
    bit ack;

    for (i = 0; i < 8; i++) {
        SDA = (b & 0x80) ? 1 : 0;   /* data changes only while SCL is low */
        b <<= 1;
        half_bit();
        SCL = 1;
        half_bit();
        SCL = 0;
    }

    SDA = 1;                        /* release SDA for the slave */
    half_bit();
    SCL = 1;
    half_bit();
    ack = !SDA;
    SCL = 0;
    return ack;
}

/* Shift in 8 bits MSB first, then send ACK (more to read) or NACK (last byte). */
unsigned char i2c_read(bit ack)
{
    unsigned char i, b = 0;

    SDA = 1;                        /* release SDA so the slave can drive it */
    for (i = 0; i < 8; i++) {
        half_bit();
        SCL = 1;
        half_bit();
        b = (unsigned char)((b << 1) | (SDA ? 1 : 0));
        SCL = 0;
    }

    SDA = ack ? 0 : 1;
    half_bit();
    SCL = 1;
    half_bit();
    SCL = 0;
    SDA = 1;
    return b;
}
