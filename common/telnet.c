#include <assert.h>

#include "log.h"
#include "telnet.h"

#define BUFSIZE 32


/* takes in a buffer from a telnet server, it will return the processed
 * buffer in out, stripping or handling commands as necessary, returning
 * all IAC commands in COMMANDS */
int
process_commands(const unsigned char *in, unsigned char *out, Command *commands)
{
	unsigned const char *start;
	int cmd_count = 0;
	int option_count = 0;

	for (start = in; *start; ++start) {
		if (*start < 128) {
			*out = *start;
			++out;
			continue;
		}

		while (*start > 127) {
			if (*start == IAC) {
				++start;
				commands[cmd_count].command = *start;
				if (*start >= WILL && *start <= DONT) {
					++start;
					commands[cmd_count].option[0] = *start;
					commands[cmd_count].opt_count = 1;
				} else if (*start == SB) {
					++start;
					while (*start != IAC || *(start+1) != SE){
						commands[cmd_count].option[option_count] = *start;
						++start;
						++option_count;
						assert(option_count < 16);
					}
					++start;
					commands[cmd_count].opt_count = option_count;
				} else {
					commands[cmd_count].option[0] = 0;
					commands[cmd_count].opt_count = 0;
				}
				++cmd_count;
			} else {
				LOG_INFO("unhandled byte: %d", *start);
			}
			++start;
		}
		--start;
	}
	*out = '\0';

	return cmd_count;
}
