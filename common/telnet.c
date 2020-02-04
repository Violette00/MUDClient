#include <stdio.h>

#include "telnet.h"
#include "log.h"

#define BUFSIZE 32


/* takes in a buffer from a telnet server, it will return the processed
 * buffer in out, stripping or handling commands as necessary, returning
 * all IAC commands in COMMANDS */
int
process_commands(const unsigned char *in, unsigned char *out, Command *commands)
{
	unsigned const char *start;
	unsigned char bytes[BUFSIZE];
	int i;

	for (start = in; *start; start++) {
		if (*start < 128) {
			*(out++) = *start;
			continue;
		}

		for (i = 0; i < BUFSIZE && *start > 127; i++, start++) {
			if (*start == IAC) {
				bytes[i++] = *(start++);
				bytes[i++] = *(start++);
			}
			bytes[i++] = *(start++);
		}

		if (i == BUFSIZE)
			LOG_ERR("OVERRAN BUFFER");
		LOG_INFO_NONL("got bytes [");
		for (; i > 0; i--) {
			fprintf(stderr, " %d", bytes[BUFSIZE - i]);
			if (i > 1)
				fprintf(stderr, ",");
		}
		fprintf(stderr, " ]\n");
	}

	return 0;
}
