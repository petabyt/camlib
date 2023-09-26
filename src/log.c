// Optional implementation of a logging mechanism - you can replace this file
// with your own mechanism.

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <camlib.h>

void ptp_verbose_log(char *fmt, ...) {
	#ifdef VERBOSE
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	#endif
}

void ptp_panic(char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	printf("PTP triggered PANIC\n");
	exit(1);
}
