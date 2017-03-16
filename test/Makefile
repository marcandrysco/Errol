## Parameters

CC      = gcc
CXX     = g++
LD      = g++
CFLAGS  = -g -O2 -Wall -Werror -I../lib
CXXFLAGS= -g -O2 -std=c++11 -Wall -Werror -I../lib -I../grisu/include
LDFLAGS = -L../lib -L../grisu/lib -static -lerrol -ldouble-conversion -lm -lgmp
AR      = ar rcs

BIN  = errol-test
OBJ  = main.o proof.o interop.o dragon4.o
SRC  = main.c proof.c interop.cpp dragon4.c
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
proof.o: proof.c Makefile $(INC) $(DEP)
interop.o: interop.cpp Makefile $(INC) $(DEP)
dragon4.o: dragon4.c Makefile $(INC) $(DEP)

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
