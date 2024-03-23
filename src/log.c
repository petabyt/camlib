// Optional implementation of a logger - you can and should replace this file.

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

__attribute__ ((noreturn))
void ptp_panic(char *fmt, ...) {
	printf("PTP abort: ");
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	abort();
}
