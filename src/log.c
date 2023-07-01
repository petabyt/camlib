// Optional implementation of a logging mechanism - you can replace this file
// with your own mechanism.

#include <stdio.h>
#include <stdarg.h>

#include <camlib.h>

void ptp_verbose_log(char *fmt, ...) {
	#ifndef VERBOSE
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	#endif
}
