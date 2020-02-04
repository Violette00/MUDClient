#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void
logmsg(const char *file, int line, const char *level, char nl,
		const char *format, ...)
{
	va_list ap;
	time_t now;

	va_start(ap, format);
	time(&now);
	fprintf(stderr, "%s (%s:%d) [%s] ", ctime(&now), file, line, level);
	vfprintf(stderr, format, ap);
	if (nl)
		fprintf(stderr, "\n");
	va_end(ap);
}
