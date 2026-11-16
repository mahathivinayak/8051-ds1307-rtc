/*
 * Host-side tests for the DS1307 driver.
 *
 * The real i2c.c drives 8051 port pins, so here the four I2C functions are
 * replaced by a mock that behaves like a DS1307 on the bus: an 8-byte
 * register file with an auto-incrementing register pointer.
 *
 *   make test
 */
#include <stdio.h>
#include <string.h>

#define bit int                    /* Keil C51 'bit' type -> int on the host */
#include "../src/ds1307.h"
#include "../src/i2c.h"

static int failures, checks;
#define CHECK(cond)                                                              \
    do {                                                                         \
        checks++;                                                                \
        if (!(cond)) {                                                           \
            failures++;                                                          \
            printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);               \
        }                                                                        \
    } while (0)

/* ---------------------------------------------------------------- mock DS1307 */
static unsigned char reg[8];
static int present = 1;
static enum { IDLE, ADDR, POINTER, WRITING, READING } state = IDLE;
static unsigned char ptr;

void i2c_init(void) { state = IDLE; }
void i2c_start(void) { state = ADDR; }
void i2c_stop(void) { state = IDLE; }

bit i2c_write(unsigned char b)
{
    switch (state) {
    case ADDR:
        if (!present || (b & 0xFE) != 0xD0)
            return 0;                                  /* nobody ACKs */
        state = (b & 1) ? READING : POINTER;
        return 1;
    case POINTER:
        ptr = b & 7;
        state = WRITING;
        return 1;
    case WRITING:
        reg[ptr] = b;
        ptr = (ptr + 1) & 7;
        return 1;
    default:
        return 0;
    }
}

unsigned char i2c_read(bit ack)
{
    unsigned char b = reg[ptr];
    (void)ack;
    ptr = (ptr + 1) & 7;
    return b;
}

/* ---------------------------------------------------------------- tests */
static void test_bcd(void)
{
    unsigned char v;
    int ok = 1;

    for (v = 0; v < 100; v++)
        if (bcd_to_bin(bin_to_bcd(v)) != v)
            ok = 0;
    CHECK(ok);
    CHECK(bin_to_bcd(59) == 0x59);
    CHECK(bcd_to_bin(0x23) == 23);
}

static void test_set_and_read(void)
{
    rtc_time_t in = { 5, 30, 14, 2, 16, 11, 26 };     /* 14:30:05 MON 16/11/2026 */
    rtc_time_t out;

    memset(reg, 0xFF, sizeof reg);
    present = 1;

    CHECK(ds1307_set(&in) == DS1307_OK);
    CHECK(reg[0] == 0x05);                             /* CH bit cleared -> clock running */
    CHECK(reg[1] == 0x30);
    CHECK(reg[2] == 0x14);                             /* 24-hour mode */
    CHECK(reg[3] == 2);
    CHECK(reg[4] == 0x16 && reg[5] == 0x11 && reg[6] == 0x26);
    CHECK(reg[7] == 0x00);                             /* square-wave output off */

    CHECK(ds1307_read(&out) == DS1307_OK);
    CHECK(memcmp(&in, &out, sizeof in) == 0);
}

static void test_clock_halted(void)
{
    rtc_time_t t;

    /* Fresh chip / battery removed: CH = 1 and the seconds register is not counting. */
    reg[0] = 0x80 | 0x42;
    CHECK(ds1307_read(&t) == DS1307_HALTED);
    CHECK(t.sec == 42);                                /* CH bit masked out of the value */
}

static void test_12_hour_mode(void)
{
    rtc_time_t t;

    reg[0] = 0x00;
    reg[2] = 0x40 | 0x20 | 0x02;                       /* 12-hour mode, PM, 2 -> 14:00 */
    CHECK(ds1307_read(&t) == DS1307_OK && t.hour == 14);

    reg[2] = 0x40 | 0x12;                              /* 12 AM -> 00 */
    ds1307_read(&t);
    CHECK(t.hour == 0);

    reg[2] = 0x40 | 0x20 | 0x12;                       /* 12 PM -> 12 */
    ds1307_read(&t);
    CHECK(t.hour == 12);
}

static void test_not_present(void)
{
    rtc_time_t t = { 1, 2, 3, 4, 5, 6, 7 };
    rtc_time_t before = t;

    present = 0;
    CHECK(ds1307_read(&t) == DS1307_NO_ACK);
    CHECK(memcmp(&t, &before, sizeof t) == 0);         /* output untouched */
    CHECK(ds1307_set(&t) == DS1307_NO_ACK);
    present = 1;
}

int main(void)
{
    test_bcd();
    test_set_and_read();
    test_clock_halted();
    test_12_hour_mode();
    test_not_present();

    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
