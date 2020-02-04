#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define TIMEBUFSIZE 36

void
logmsg(const char *file, int line, const char *level, char nl,
		const char *format, ...)
{
	va_list ap;
	time_t now;
	char timebuf[TIMEBUFSIZE];

	va_start(ap, format);
	time(&now);
	strftime(timebuf, TIMEBUFSIZE, "%FT%TZ", gmtime(&now));
	fprintf(stderr, "%s (%s:%d) [%s] ", timebuf, file, line, level);
	vfprintf(stderr, format, ap);
	if (nl)
		fprintf(stderr, "\n");
	va_end(ap);
}
