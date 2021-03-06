#include <stddef.h>
#include <stdint.h>
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"

//#include <stdio.h>
//#include <string.h>
//#include <termios.h>

/* remove when we make this a separate program */
int ls(char *path);

static int parse_input(char *string);

#define VERSION "10.0"

int main(int argc, char **argv) {

	char input_string[256];
	int input_pointer,result;
	int ch,done=0;

	struct termios oldt, newt;

//	register long sp asm ("sp");

//	printf("Our sp=%x\n",sp);

	tcgetattr( 0, &oldt);
	newt = oldt;

	cfmakeraw(&newt);

	tcsetattr(0, TCSANOW, &newt);

	while (1) {
		input_pointer=0;

		printf("] ");

		while(1) {

			while(1) {
				ch=getchar();
				if (ch) break;

				asm volatile(
					"mov r1,#65536\n"
					"a_loop:\n"
					"subs   r1,r1,#1\n"
					"bne    a_loop\n"
					::: "r1", "memory");

			}

//			printf("VMW: input %d\n",input_pointer);

//			printf("VMW: %d\n",ch);

			if ((ch=='\n') || (ch=='\r')) {
				printf("\n");
				input_string[input_pointer]=0;
				result=parse_input(input_string);
				if (result==1) done=1;
				break;
			}

			/* ctrl-D? */
			if (ch==4) {
				done=1;
				break;
			}

			/* Backspace */
			if ((ch==0x7f) || (ch=='\b')) {

				if (input_pointer>0) {
					input_pointer--;
					printf("\b \b");
				}
			}
			else {
				input_string[input_pointer]=ch;
				input_pointer++;
				putchar(ch);
			}
		}
		if (done) break;
	}
	tcsetattr( 0, TCSANOW, &oldt);

	return 0;
}


static int print_help(void) {

	printf("VMWos Shell Version %s\n\n",VERSION);
	printf("\tblink on/off	- turns on/off heartbeat LED\n");
	printf("\tcat X		- dump file to screen\n");
	printf("\tcls		- clears the screen\n");
	printf("\tcolor X	- set text to color #X\n");
	printf("\techo X	- prints string X\n");
	printf("\tfont X	- sets the font to font #X\n");
	printf("\tgetpid	- print current process ID\n");
	printf("\tgradient	- make background look cool\n");
	printf("\thelp		- prints this help message\n");
	printf("\tls		- list files\n");
	printf("\trandom	- print random number\n");
	printf("\treset		- reset the machine\n");
	printf("\trun X		- run program #X\n");
	printf("\tstop X	- stop program #X\n");
	printf("\ttemp		- print the temperature\n");
	printf("\ttime		- print seconds since boot\n");
	printf("\ttb1		- play TB1\n");
	printf("\tver		- print version\n");
	printf("\n");

	return 0;
}

static int parse_input(char *string) {

	int result=0;

	if (!strncmp(string,"echo",4)) {
		printf("%s\n",string+5);
	}
	else if (!strncmp(string,"cat",3)) {
		int fd,st_result;
		char buffer[128];

		fd=open(string+4,O_RDONLY,0);
		if (fd<0) {
			printf("Error opening file %s\n",string+4);
		}
		else {
			while(1) {
				st_result=read(fd,buffer,128);
				if (st_result<1) break;
				write(2,buffer,st_result);
			}
			close(fd);
		}
	}
	else if (!strncmp(string,"cls",3)) {
		printf("\n\r\033[2J\n");
	}
	else if (!strncmp(string,"font ",5)) {
		vmwos_setfont(string[5]);
	}
	else if (!strncmp(string,"gradient",8)) {
		vmwos_gradient();
	}
	else if (!strncmp(string,"getpid",6)) {
		printf("Current pid: %d\n",getpid());
	}
	else if (!strncmp(string,"exit",4)) {
		result=1;
	}
	else if (!strncmp(string,"blink ",6)) {
		vmwos_blink(string[6]);
	}
	else if (!strncmp(string,"help",4)) {
		result=print_help();
	}
	else if (!strncmp(string,"run ",4)) {
		result=vmwos_run(string[4]);
	}
	else if (!strncmp(string,"stop ",5)) {
		result=vmwos_stop(string[5]);
	}
	else if (!strncmp(string,"color ",6)) {
		printf("%c[3%cm\n",27,string[6]);
	}
	else if (!strncmp(string,"ver",3)) {
		printf("VMWos Shell version %s\n",VERSION);
	}
	else if (!strncmp(string,"time",4)) {
		printf("Time since boot: %ds\n",time(NULL));
	}
	else if (!strncmp(string,"reset",5)) {
		printf("Resetting...\n");
		reboot();
	}
	else if (!strncmp(string,"temp",4)) {
		uint32_t temperature;
		temperature=vmwos_get_temp();
		printf("Current temperature %dC, %dF\n",
			temperature/1000,
			((temperature*9)/5000)+32);
	}
	else if (!strncmp(string,"tb1",3)) {
		vmwos_tb1();
	}
	else if (!strncmp(string,"ls",2)) {
		ls(".");
	}
	else if (!strncmp(string,"random",6)) {
		printf("%d\n",rand());
	}
	else {
		printf("\nUnknown commmand: \"%s\"!\n",string);
	}

	return result;
}



