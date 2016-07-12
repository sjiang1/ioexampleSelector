#Funcwatch Makefile
#This variable specifies the architecture
#Supported values are x86.

CC = gcc
CFLAGS = -std=gnu99 -g -I./includes -w -O0
LDFLAGS = -g 
LDLIBS = -lz

ioexampleSelector:ioexampleSelector.o vector.o parameter.o util.o globals.o libs/libcsv.a

tests/test_primitive_0_int: 	tests/test_primitive_0_int.o
tests/test_primitive_0_int_1: 	tests/test_primitive_0_int_1.o

clean:
	rm -f ioexampleSelector `find -name \*.o` *~ \
		tests/test_primitive_0_int_1 \
		tests/test_primitive_0_int
