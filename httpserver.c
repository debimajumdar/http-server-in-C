#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "asgn2_helper_funcs.h"
#include "functions.h"

int main(int argc, char **argv) {

    if (argc < 2) {

        fprintf(stderr, "need more!!!\n");

        exit(1);
    }

    Listener_Socket sk;

    // To get port number
    int port;
    sscanf(argv[1], "%d", &port);

    // The wrong port number and exit with code 1
    if (port < 1 || port > 65535) {

        fprintf(stderr, "wrong port number\n");

        exit(1);
    }

    if (listener_init(&sk, port) == -1) {

        fprintf(stderr, "socket initialize failed\n");

        exit(1);
    }

    // The port number is true
    while (1) {

        int sd = listener_accept(&sk);

        if (sd < 0) {

            fprintf(stderr, "socket accept failed\n");

            exit(1);

        } else {

            read_request(sd);
        }

        close(sd);
    }

    return 0;
}
