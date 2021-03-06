#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "lib/string.h"

int strncmp(const char *s1, const char *s2, uint32_t n) {

	int i=0,r;

	while(1) {

		if (i==n) return 0;

		r=s1[i]-s2[i];
		if (r!=0) return r;

		i++;
	}

	return 0;
}

int memcmp(const char *s1, const char *s2, uint32_t n) {

	int i=0,r;

	while(1) {

		if (i==n) return 0;

		r=s1[i]-s2[i];
		if (r!=0) return r;

		i++;
	}

	return 0;
}

char *strncpy(char *dest, const char *src, uint32_t n) {

	uint32_t i;

	for(i=0; i<n; i++) {
		dest[i]=src[i];
		if (src[i]=='\0') break;
	}

	return dest;

}

int strlen(const char *s1) {

	int i=0;

	while(s1[i]!=0) i++;

	return i;

}

