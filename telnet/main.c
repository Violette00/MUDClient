#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


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

    return 0;
}

