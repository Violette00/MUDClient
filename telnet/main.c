#include <assert.h>
#include <err.h>
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

#define MAX_EVENTS 1
#define BUFSIZE 2048

static void usage(void);
static int mainloop(int socket);
static int connect_to_host(const char *hostname, const char *port);

static char *argv0;

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

static int
mainloop(int socket)
{
	struct pollfd pfds[2];
	int count;
	unsigned char buffer[BUFSIZE], out[BUFSIZE];

	pfds[0].fd = socket;
	pfds[0].events = POLLIN;

	pfds[1].fd = STDIN_FILENO;
	pfds[1].events = POLLIN;

	out[0] = buffer[0] = '\0';
	while (poll(pfds, 2, -1) >= 0) {
		/* end loop on either hangup */
		if (pfds[0].revents & POLLHUP || pfds[1].revents & POLLHUP)
			return 0;

		if (pfds[0].revents & POLLERR || pfds[1].revents & POLLERR)
			abort();

		if (pfds[0].revents & POLLNVAL || pfds[1].revents & POLLNVAL)
			abort();

		if (pfds[0].revents & POLLIN) {
			count = read(socket, buffer, BUFSIZE);
			LOG_INFO("got %d bytes", count);
			if (count == 0)
				return 0;
			assert(count >= 0 && count != BUFSIZE);
			buffer[count] = '\0';
			process_commands(buffer, out, NULL);
			printf("%s", out);
		}

		if (pfds[1].revents & POLLIN) {
			count = read(STDIN_FILENO, buffer, BUFSIZE);
			assert(count >= 0 && count < BUFSIZE);
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

