include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	fat16.a

fat16.a:	fat16.o
	$(CROSS)$(AR) crs fat16.a fat16.o

fat16.o:	fat16.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o fat16.o -c fat16.c

clean:
	rm -f *~ *.o *.a

