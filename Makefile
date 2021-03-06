include Makefile.inc

all:	kernel/kernel.img

kernel/kernel.img:
	cd userspace && make image
	cd kernel && make

clean:	
	cd kernel && make clean
	cd userspace && make clean
	cd tools && make clean
	rm -f *~

