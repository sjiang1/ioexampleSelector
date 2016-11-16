#Funcwatch Makefile
#This variable specifies the architecture
#Supported values are x86.

CC = gcc
CFLAGS = -std=gnu99 -g -I./includes -w -O0
LDFLAGS = -g
LDLIBS = -lz -lpthread
current_dir = $(shell pwd)

default: ioselect

ioselect: ioexampleSelector.o vector.o parameter.o util.o globals.o libs/*.a $(current_dir)/libs/*.so
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f ioselect `find -name \*.o` *~ 
