.PHONY: build test clean

BIN=clarawm
CFLAGS=-lX11 -O2 -Wall -Werror
CC=clang
CSRC=$(wildcard *.c)

build: $(BIN)

$(BIN): $(CSRC)
	$(CC) $(CSRC) -o $(BIN) $(CFLAGS)

test:
	Xephyr -ac -screen 800x600 -br -reset -terminate :1 &
	sleep .5
	DISPLAY=:1 ./$(BIN)

clean:
	rm $(BIN)
