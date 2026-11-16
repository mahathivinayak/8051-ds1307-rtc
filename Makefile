# Host-side tests for the DS1307 driver (no board needed).
# The firmware itself is built in Keil uVision (C51) - see README.md.

CC      ?= gcc
CFLAGS  ?= -std=c99 -Wall -Wextra -Werror -O2 -Dbit=int
SAN     := -fsanitize=address,undefined -fno-sanitize-recover=all -g

.PHONY: test test-san clean

test: build/test_ds1307
	./build/test_ds1307

test-san: build/test_ds1307_san
	./build/test_ds1307_san

build/test_ds1307: test/test_ds1307.c src/ds1307.c src/ds1307.h src/i2c.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ test/test_ds1307.c src/ds1307.c

build/test_ds1307_san: test/test_ds1307.c src/ds1307.c src/ds1307.h src/i2c.h
	@mkdir -p build
	$(CC) $(CFLAGS) $(SAN) -o $@ test/test_ds1307.c src/ds1307.c

clean:
	rm -rf build
