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

	for (start = in; *start; start++) {
		if (*start < 128) {
			*(out++) = *start;
			continue;
		}

		LOG_INFO_NONL("got bytes [");
		while (*start > 127) {
			if (*start == IAC) {
				fprintf(stderr, "%d, %d, ", *start,
						*(start+1));
				start += 2;
			}
			fprintf(stderr, "%d, ", *start);
			start++;
		}
		fprintf(stderr, "]\n");
	}

	return 0;
}
