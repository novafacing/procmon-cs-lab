CC=gcc
GCFLAGS=$(shell pkg-config --cflags gtk+-3.0)
GLDFLAGS=$(shell pkg-config --libs gtk+-3.0)
LDFLAGS=-lcpuid
CFLAGS=-g -rdynamic -std=gnu11
WFLAGS=-Wall -Werror -Wextra -Wno-unused-parameter -Wno-implicit-function-declaration
SANLFLAGS=-lasan
SANCFLAGS=-fsanitize=address -fno-omit-frame-pointer

all: procmon
sanitize: procmon-san testing-san
testing: test

procmon: procmon.c ui.c utils.c
	$(CC) $(CFLAGS) $(GCFLAGS) -o $@ $^ $(GLDFLAGS) $(LDFLAGS) $(WFLAGS)

test: test.c utils.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(WFLAGS)

procmon-san: procmon.c ui.c utils.c
	$(CC) $(CFLAGS) $(GCFLAGS) $(SANCFLAGS) -o $@ $^ $(GLDFLAGS) $(LDFLAGS) $(WFLAGS) $(SANLFLAGS)

testing-san: test.c utils.c
	$(CC) $(CFLAGS) $(SANCFLAGS) -o $@ $^ $(LDFLAGS) $(WFLAGS) $(SANLFLAGS)
