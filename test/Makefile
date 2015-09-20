## Parameters

CC      = gcc
CXX     = g++
LD      = gcc
CFLAGS  = -g -O2 -Wall -Werror
LDFLAGS = -lm -L../lib -L../grisu -static -lerrol -ldouble-conversion -lm -lgmp
AR      = ar rcs

BIN  = errol-test
OBJ  = main.o interop.o dragon4.o
SRC  = main.c interop.cpp dragon4.c
INC  = dragon4.h
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

interop.o: interop.cpp Makefile $(INC) $(DEP)
	$(CXX) -c $< -o $@ $(CFLAGS)

dragon4.o: dragon4.c Makefile $(INC) $(DEP)
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
