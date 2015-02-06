## Parameters

CC      = gcc
LD      = gcc
CFLAGS  = -g -O2 -Wall -Werror -fpic
LDFLAGS = -lm
AR      = ar rcs

LIB  = liberrol.a
DYN  = liberrol.so
OBJ  = errol.o
SRC  = errol.c
INC  = errol.h
DIST = Makefile $(SRC) $(INC)
VER  = 1.0
PKG  = errol

## Build rules

all: $(LIB) $(DYN)

$(LIB): $(OBJ)
	$(AR) $@ $^

$(DYN): $(OBJ)
	$(LD) $^ -o $@ -shared $(LDFLAGS)

errol.o: errol.c Makefile $(INC)
	$(CC) -c $< -o $@ $(CFLAGS)

## Clean rules

clean:
	rm -rf liberrol.a liberrol.so $(OBJ)

## Distribute rules

dist:
	rm -rf $(PKG)
	mkdir $(PKG)
	cp $(DIST) $(PKG)
	tar -zcf $(PKG).tar.gz $(PKG)
	rm -rf $(PKG)

## Phony

.PHONY: all clean dist
