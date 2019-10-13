
#include "include.h"
#include <stdarg.h>
#include <stdio.h>

/*****************************/
void output(const char* description, ...)
{
	/* Format the description */
	va_list vl;
	va_start(vl, description);

	fprintf(stdout, "-- ");
	vfprintf(stdout, description, vl);

	va_end(vl);

	/* Append newline and flush */
	fputc('\n', stdout);
	fflush(stdout);
}

/*****************************/
void throw_error(const char* description, ...)
{
	/* Format the error */
	va_list vl;
	va_start(vl, description);

	fprintf(stderr, "ERROR -- ");
	vfprintf(stderr, description, vl);

	va_end(vl);

	/* Append newline and flush */
	fputc('\n', stderr);
	fflush(stderr);
}
