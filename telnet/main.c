#include <assert.h>
#include <err.h>
#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../common/telnet.h"
#include "../common/log.h"

#define MAX_EVENTS 2
#define BUFSIZE 2048
#define COMMAND_COUNT 32

typedef struct {
	char will_will, will_do;
	void (*handle_sb)(int, const Command*);
} CommandHandler;

static void usage(void);
static int mainloop(int socket);
static int connect_to_host(const char *hostname, const char *port);
static void handle_commands(int socket, const Command *cmds, int ncmds,
		CommandHandler *handlers);
static void handle_will(unsigned char *buffer, unsigned char option,
		const CommandHandler *handlers);
static void handle_do(unsigned char *buffer, unsigned char option,
		const CommandHandler *handlers);
static void handle_subnegotiation(int socket, const Command *cmd,
		CommandHandler *handlers);

static char *argv0;

static void
handle_tt_sb(int socket, const Command *cmd)
{
	unsigned char buffer[BUFSIZE];
	char *ttype;
	size_t ttype_len;
	unsigned i;

	assert(cmd->opt_count == 2);
	assert(cmd->options[1] == 1);

	buffer[0] = IAC;
	buffer[1] = SB;
	buffer[2] = 24; /* terminal-type */
	buffer[3] = 0; /* is */
	ttype = getenv("TERM");
	ttype = ttype ? ttype : "screen";	/* default to screen */
	LOG_INFO("using terminal type %s", ttype);
	ttype_len = strlen(ttype);
	memcpy(buffer + 4, ttype, ttype_len);
	buffer[ttype_len + 4] = IAC;
	buffer[ttype_len + 5] = SE;

	LOG_INFO_NONL("sending terminal-type reply: [ ");
	for (i = 0; i < ttype_len + 6; ++i)
		fprintf(stderr, "%d ", buffer[i]);
	fprintf(stderr, "]\n");

	assert(write(socket, buffer, ttype_len + 6) > 0);
}

static void
handle_will(unsigned char *buffer, unsigned char option,
		const CommandHandler *handlers) {
	buffer[0] = IAC;
	if (handlers[option].will_do) {
		LOG_INFO("replying DO to %d", option);
		buffer[1] = DO;
	} else {
		LOG_INFO("replying DONT to %d", option);
		buffer[1] = DONT;
	}
	buffer[2] = option;
}

static void
handle_do(unsigned char *buffer, unsigned char option,
		const CommandHandler *handlers) {
	buffer[0] = IAC;
	if (handlers[option].will_will) {
		LOG_INFO("replying WILL to %d", option);
		buffer[1] = WILL;
	} else {
		LOG_INFO("replying WONT to %d", option);
		buffer[1] = WONT;
	}
	buffer[2] = option;
}

static void
handle_subnegotiation(int socket, const Command *cmd, CommandHandler *handlers)
{
	unsigned i;
	unsigned char option = cmd->options[0];

	LOG_INFO_NONL("Got subnegotiation: %d [", option);
	for (i = 1; i < cmd->opt_count; ++i)
		fprintf(stderr, " %d", cmd->options[i]);
	fprintf(stderr, " ]\n");
	if (handlers[option].handle_sb)
		handlers[option].handle_sb(socket, cmd);
	else
		LOG_INFO("ignoring option %d", option);
}

static void
handle_commands(int socket, const Command *cmds, int ncmds,
		CommandHandler *handlers)
{
	unsigned char buffer[BUFSIZE];

	for ( ; ncmds > 0; ncmds--, cmds++) {
		LOG_INFO("processing command %d", cmds->command);
		if (cmds->command == WILL) {
			handle_will(buffer, cmds->options[0], handlers);
			assert(write(socket, buffer, 3) == 3);
		} else if (cmds->command == DO) {
			handle_do(buffer, cmds->options[0], handlers);
			assert(write(socket, buffer, 3) == 3);
		} else if (cmds->command == SB){
			handle_subnegotiation(socket, cmds, handlers);
		} else if (cmds->command == GA) {
			/* eat Go Ahead as nobody cares */
		} else {
			LOG_INFO("ignoring command: %d", cmds->command);
		}
	}
}

static void
usage(void)
{
    fprintf(stderr, "usage: %s {HOST} {PORT}\n", argv0);
}

static int
connect_to_host(const char *hostname, const char *port)
{
    struct addrinfo hints, *res, *res0;
    int error;
    int s;
    const char *cause = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(hostname, port, &hints, &res0);

    if (error) {
        errx(1, "%s", gai_strerror(error));
        /*NOTREACHED*/
    }

    s = -1;
    for (res = res0; res; res = res->ai_next){
        s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (s < 0){
            cause = "socket";
            continue;
        }

        if (connect(s, res->ai_addr, res->ai_addrlen) < 0){
            cause = "connect";
            close(s);
            s = -1;
            continue;
        }

        break; /* okay we got one */
    }

    if (s < 0){
        err(1, "%s", cause);
    }

    freeaddrinfo(res0);
    return s;
}

static void
set_handlers(CommandHandler *handlers)
{
	handlers[24].will_will = 1;
	handlers[24].handle_sb = handle_tt_sb;
}

static int
mainloop(int socket)
{
	struct pollfd pfds[MAX_EVENTS];
	int count, cmd_count;
	unsigned char buffer[BUFSIZE], out[BUFSIZE];
	Command commands[COMMAND_COUNT];
	CommandHandler handlers[UCHAR_MAX];

	memset(handlers, 0, sizeof(CommandHandler) * UCHAR_MAX);
	set_handlers(handlers);

	pfds[0].fd = socket;
	pfds[0].events = POLLIN;

	pfds[1].fd = STDIN_FILENO;
	pfds[1].events = POLLIN;

	out[0] = buffer[0] = '\0';
	while (poll(pfds, MAX_EVENTS, -1) >= 0) {
		/* end loop on either end hangup */
		if (pfds[0].revents & POLLHUP || pfds[1].revents & POLLHUP)
			return 0;

		if (pfds[0].revents & POLLERR || pfds[1].revents & POLLERR)
			abort();

		if (pfds[0].revents & POLLNVAL || pfds[1].revents & POLLNVAL)
			abort();

		if (pfds[0].revents & POLLIN) {
			count = read(socket, buffer, BUFSIZE - 1);
			LOG_INFO("got %d bytes", count);
			if (count == 0)
				return 0;
			buffer[count] = '\0';
			cmd_count = process_commands(buffer, out, commands);
			handle_commands(socket, commands, cmd_count, handlers);
			printf("%s", out);
		}

		if (pfds[1].revents & POLLIN) {
			count = read(STDIN_FILENO, buffer, BUFSIZE - 1);
			if (count == 0)
				return 0;
			buffer[count] = '\0';
			LOG_INFO_NONL("writting %s", buffer);
			count = write(socket, buffer, count);
			assert(count >= 0);
		}
	}

	abort();
}

int
main(int argc, char **argv)
{
    int s;

    argv0 = argv[0];
    if (argc < 3) {
        fprintf(stderr, "not enough arguments provided\n");
        usage();
        return 1;
    }

    s = connect_to_host(argv[1], argv[2]);
    mainloop(s);

    return 0;
}
