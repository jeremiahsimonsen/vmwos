#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "lib/printk.h"

#include "time.h"
#include "arch/arm1176/arm1176-mmu.h"

#define MEMORY_BENCHMARK 1

#if (MEMORY_BENCHMARK==1)

void *memset_byte(void *s, int c, uint32_t n) {

	uint32_t i;
	char *b;

	b=(char *)s;

	for(i=0;i<n;i++) b[i]=c;

	return s;
}


void *memset_4byte(void *s, int c, uint32_t n) {

	uint32_t i;
	uint32_t *b;
	uint8_t *ch;
	uint32_t pattern;
	uint32_t offset;
	uint32_t count;


	pattern=(c&0xff);
	pattern=pattern|(pattern<<8)|(pattern<<16)|(pattern<<24);

	//printk("Writing %x (4 x %x)\n",pattern,c);

	offset=((uint32_t)s)%4;

	ch=(uint8_t *)s;

	/* Do leading edge if unaligned */
	if (offset) {
		for(i=0;i<(4-offset);i++) ch[i]=c;
		b=(uint32_t *)(ch+(4-offset));
		count=n-1;
	}
	else {
		b=(uint32_t *)s;
		count=n;
	}

	/* Do 4-byte chunks */
	for(i=0;i<(count/4);i++) b[i]=pattern;

	/* Do trailing edge if unaligned */
	for(i=0;i<offset;i++) {
//		printk("Tail: setting offset %d\n",(n-offset)+i);
		ch[(n-offset)+i]=c;
	}

	return s;
}

#endif

/* based on the version in the Linux kernel */
/* arch/arm/lib/memset.S */
//void *memset_64byte(void *s, int c, uint32_t n) {
void *memset(void *s, int c, uint32_t n) {

	asm("stmfd	sp!, {r4-r8, lr}");	// save registers on stack

	// check if address unaligned
	asm("ands	r3, r0, #3");
	// we return the pointer to the buffer area (memset definition)
	asm("mov	ip, r0");		// ip is r9 = scratch reg

	asm("beq	asm_memset_4bytealigned");

	asm("subs	r2, r2, #4");	// are we copying more than 4?
	asm("blt	asm_memset_lessthan4");	// if not we are done

        asm("cmp	r3, #2");
	asm("strltb	r1, [ip], #1");	// if <2 store 1 byte
	asm("strleb	r1, [ip], #1"); // if <=2 store another byte
	asm("strb	r1, [ip], #1"); // always store at least one byte
	asm("add	r2, r2, r3");   // adjust count (r2 = r2 - (4 - r3))

	asm("asm_memset_4bytealigned:");

	// fill r1 with byte pattern
	asm("orr	r1, r1, r1, lsl #8");	// copy low 8-bits up 8-bits
	asm("orr	r1, r1, r1, lsl #16");	// copy low 16-bits to top

	// going to write 32 bytes at a time
	asm("mov	r3, r1");
	asm("mov	r4, r1");
        asm("mov	r5, r1");
        asm("mov	r6, r1");
        asm("mov	r7, r1");
        asm("mov	r8, r1");
        asm("mov	lr, r1");

	/* Linux checks if count >96 and address > 0xc000 0000	*/
	/* And does something special ?				*/

//	asm("cmp	r2, #96");
//	asm("tstgt	ip, #31");
//	asm("ble	3f");


	asm("asm_memset_loop:");

	// decrement count by 64
	asm("subs	r2, r2, #64");

	// if was greater than 64, write out 64-bytes of value
	asm("stmgeia	ip!, {r1, r3-r8, lr}");
	asm("stmgeia	ip!, {r1, r3-r8, lr}");

	// loop until less than 64
	asm("bgt	asm_memset_loop");


	// if count is 0 we are done
	asm("beq	asm_memset_done");

	// otherwise, need to clean up

	// handle 32-64 case
	asm("tst	r2, #32");
	asm("stmneia	ip!, {r1, r3-r8, lr}");

	/* handle 16-32 case */
	asm("tst	r2, #16");
	asm("stmneia	ip!, {r4-r7}");

	/* handle 8-16 case */
	asm("tst	r2, #8");
	asm("stmneia	ip!, {r1, r3}");

	/* handle 4-8 case */
	asm("tst	r2, #4");
	asm("strne	r1, [ip], #4");

	// We have less than 4 bytes left
	asm("asm_memset_lessthan4:");

	asm("tst	r2, #2");		// if 2 or 3, write 2 out
	asm("strneb	r1, [ip], #1");
	asm("strneb	r1, [ip], #1");
	asm("tst	r2, #1");
	asm("strneb	r1, [ip], #1");		// if 1 or 3, write 1 out

	asm("asm_memset_done:");

	asm("ldmfd	sp!, {r4-r8, lr}");	// restore regs from stack

	return s;
}


#if (MEMORY_BENCHMARK==1)

static void memset_test(void *addr, int value, int size) {

	int i,errors=0;
	char *b;
	b=(char *)addr;

	printk("\tTesting: ");
	for(i=0;i<size;i++) {
		if (b[i]!=value) {
			printk("Not match at offset %d (%x!=%x)!\n",
				i,b[i],value);
			errors++;
			if (errors>20) break;
		}
	}

	if (b[size+1]==value) {
		errors++;
		printk("Value after the end has wrong value!\n");
	}

	if (errors) printk("Failed!\n");
	else printk("Passed!\n");

}

#define BENCH_SIZE (1024*1024)
#define BENCH_ITERATIONS 16
uint8_t benchmark[BENCH_SIZE+16];

#define OFFSET 0

static void run_memory_benchmark(void) {

	int i;
	uint32_t before,after;

	before=ticks_since_boot();

	for(i=0;i<BENCH_ITERATIONS;i++) {
		memset_byte(benchmark+OFFSET,0xfe,BENCH_SIZE);
	}

	after=ticks_since_boot();

	printk("\tMEMSPEED: %d MB took %d ticks\n",
		BENCH_SIZE*BENCH_ITERATIONS,
		(after-before));

	memset_test(benchmark+OFFSET,0xfe,BENCH_SIZE);

}

static void run_memory_benchmark32(void) {

	int i;
	uint32_t before,after;

	before=ticks_since_boot();

	for(i=0;i<BENCH_ITERATIONS;i++) {
		memset_4byte(benchmark+OFFSET,0xa5,BENCH_SIZE);
	}

	after=ticks_since_boot();

	printk("\tMEMSPEED: %d MB took %d ticks\n",
		BENCH_SIZE*BENCH_ITERATIONS,
		(after-before));

	memset_test(benchmark+OFFSET,0xa5,BENCH_SIZE);
}

static void run_memory_benchmark_asm(void) {

	int i;
	uint32_t before,after;

	before=ticks_since_boot();

	for(i=0;i<BENCH_ITERATIONS;i++) {
		memset(benchmark+OFFSET,0x78,BENCH_SIZE);
	}

	after=ticks_since_boot();

	printk("\tMEMSPEED: %d MB took %d ticks\n",
		BENCH_SIZE*BENCH_ITERATIONS,
		(after-before));

	memset_test(benchmark+OFFSET,0x78,BENCH_SIZE);
}

void memory_benchmark(uint32_t memory_total) {

	/* Run some memory benchmarks */
	printk("\nRunning Memory benchmarks %x\n",benchmark+OFFSET);
	printk("Default memory:\n");
	run_memory_benchmark();

	/* Enable L1 i-cache */
	enable_l1_icache();
	printk("L1 icache enabled:\n");
	run_memory_benchmark();

	/* Enable branch predictor */
	enable_branch_predictor();
	printk("Branch predictor enabled:\n");
	run_memory_benchmark();

	/* Enable L1 d-cache */
	enable_mmu(0,memory_total);
	enable_l1_dcache();
	printk("L1 dcache enabled:\n");
	run_memory_benchmark();

	/* 32-bit version */
	printk("32-bit copy\n");
	run_memory_benchmark32();

	/* asm version */
	printk("Assembly 64-byt copy\n");
	run_memory_benchmark_asm();

}

#endif
