## Parameters

CC      = gcc
LD      = gcc
CFLAGS  = -g -O2 -Wall -Werror
LDFLAGS = -lm -L../lib -static -lerrol -lm
AR      = ar rcs

BIN  = errol-test
OBJ  = main.o
SRC  = main.c
DEP  = ../lib/liberrol.so
DIST = Makefile $(SRC) $(INC)
VER  = 1.0
PKG  = errol_test

## Build rules

all: $(BIN)

$(BIN): $(OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

main.o: main.c Makefile $(INC) $(DEP)
	$(CC) -c $< -o $@ $(CFLAGS)

## Clean rules

clean:
	rm -rf $(BIN) $(OBJ)

## Distribute rules

dist:
	mkdir $(PKG)
	cp $(DIST) $(PKG)
	tar -zcf $(PKG).tar.gz $(PKG)
	rm -rf $(PKG)

## Phony

.PHONY: all clean dist
