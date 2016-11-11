#Funcwatch Makefile
#This variable specifies the architecture
#Supported values are x86.

CC = gcc
CFLAGS = -std=gnu99 -g -I./includes -w -O0
LDFLAGS = -g 
LDLIBS = -lz

default: ioselect

ioselect: ioexampleSelector.o vector.o parameter.o util.o globals.o libs/libcsv.a
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f ioselect `find -name \*.o` *~ 
