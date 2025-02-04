/****************************************
* CSE_130 Assignment_2
* functions.c
****************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <regex.h>

#include "asgn2_helper_funcs.h"
#include "functions.h"

#define BUFFER_SIZE 4096
#define re          "([A-Z]{1,8}) /([a-zA-Z0-9.-]{1,63}) (HTTP/[0-9].[0-9])\r\n"
#define hd          "([a-zA-Z0-9.-]{1,128}): ([ -~]{0,128})\r\n"

void read_request(int sd) {

    char read_buffer[BUFFER_SIZE] = { 0 };

    int bytes_read = read(sd, read_buffer, BUFFER_SIZE);

    if (bytes_read < 0) {

        status(500, sd, 0, 0);

        return;
    }

    char *Method = NULL;
    char *URI = NULL;
    char *Version = NULL;
    char *eol = NULL;

    regex_t regex;
    regmatch_t matches[4];
    int rc = regcomp(&regex, re, REG_EXTENDED);

    rc = regexec(&regex, read_buffer, 4, matches, 0);

    if (rc == 0) {

        Method = read_buffer;
        URI = read_buffer + matches[2].rm_so;
        Version = read_buffer + matches[3].rm_so;
        eol = read_buffer + matches[3].rm_eo + 2;
        Method[matches[1].rm_eo] = '\0';
        URI[matches[2].rm_eo - matches[2].rm_so] = '\0';
        Version[matches[3].rm_eo - matches[3].rm_so] = '\0';

    } else {

        Method = NULL;
        URI = NULL;
        Version = NULL;
        eol = NULL;

        status(400, sd, 0, 0);

        return;
    }

    if (strcmp(Version, "HTTP/1.1") != 0) {

        status(505, sd, 0, 0);

        return;
    }

    char put_buffer[BUFFER_SIZE] = { 0 };
    strcpy(put_buffer, eol);

    int contlen = 0;
    regmatch_t m2[3];
    rc = regcomp(&regex, hd, REG_EXTENDED);
    int index = 0;
    int check = 0;

    while ((rc = regexec(&regex, put_buffer + index, 3, m2, 0)) == 0) {

        char key[128];
        char value[128];
        memset(key, 0, sizeof(key));
        memset(value, 0, sizeof(value));

        // Copy the key and value from the matched substrings
        strncpy(key, put_buffer + index + m2[1].rm_so, m2[1].rm_eo - m2[1].rm_so);
        strncpy(value, put_buffer + index + m2[2].rm_so, m2[2].rm_eo - m2[2].rm_so);

        // printf("key:%s\nvalue:%s\nindex:%i\n***********\n",key, value, index);
        if (strcmp(key, "Content-Length") == 0) {

            sscanf(value, "%d", &contlen);

            check += 1;
        }

        eol = put_buffer + m2[2].rm_eo + index + 2;

        // Update the current index for the next match
        index += m2[0].rm_eo;
    }

    if (eol[0] != 13 || eol[1] != 10) {

        status(400, sd, 0, 0);

        return;
    }

    regfree(&regex);

    if (strcmp(Method, "GET") == 0) {

        order_get(sd, URI);

    }

    else if (strcmp(Method, "PUT") == 0) {

        if (check < 1) {

            status(400, sd, 0, 0);

            return;
        }

        order_put(sd, URI, contlen, eol + 2);

    } else {

        status(501, sd, 0, 0);

        return;
    }

    return;
}

void status(int code, int sd, int needok, long int contlen) {

    char message[BUFFER_SIZE] = { 0 };

    if (code == 200) {

        sprintf(message, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", contlen);
        send(sd, message, strlen(message), 0);

        if (needok == 1) {

            send(sd, "OK\n", 3, 0);
        }

    } else if (code == 201) {

        send(sd, "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n", 51, 0);

    } else if (code == 400) {

        send(sd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n", 60, 0);

    } else if (code == 403) {

        send(sd, "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n", 56, 0);

    } else if (code == 404) {

        send(sd, "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n", 56, 0);

    } else if (code == 500) {

        send(sd,
            "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 23\r\n\r\nInternal Server "
            "Error\n",
            80, 0);

    } else if (code == 501) {

        send(sd, "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n", 68,
            0);

    } else if (code == 505) {

        send(sd,
            "HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion Not "
            "Supported\n",
            80, 0);
    }

    return;
}

void order_get(int sd, char *uri) {

    struct stat stat_buffer;

    if (stat(uri, &stat_buffer) == 0) {

        //check if it's a regular file
        if (S_ISREG(stat_buffer.st_mode) == 0) {

            status(403, sd, 0, 0);

            return;
        }

        int fd = open(uri, O_RDONLY, 0);

        if (fd < 0) {

            status(403, sd, 0, 0);

            return;
        }

        status(200, sd, 0, stat_buffer.st_size);
        pass_n_bytes(fd, sd, stat_buffer.st_size);

        close(fd);

        return;

    } else {

        status(404, sd, 0, 0);

        return;
    }

    return;
}

void order_put(int sd, char *uri, int contlen, char *cont) {

    struct stat stat_bufferf;

    if (stat(uri, &stat_bufferf) == 0) {

        int fd = open(uri, O_RDWR | O_CREAT | O_TRUNC, stat_bufferf.st_mode);

        if (fd < 0) {

            status(403, sd, 0, 0);

            return;

        } else {

            int bytes_written = write_n_bytes(fd, cont, strlen(cont));
            contlen -= bytes_written;
            pass_n_bytes(sd, fd, contlen);
            status(200, sd, 1, 3);
        }

        close(fd);

        return;

    } else {

        int fd = open(uri, O_RDWR | O_CREAT | O_TRUNC, 0777);

        if (fd < 0) {

            status(403, sd, 0, 0);

            return;

        } else {

            int bytes_written = write_n_bytes(fd, cont, strlen(cont));
            contlen -= bytes_written;
            pass_n_bytes(sd, fd, contlen);
            status(201, sd, 0, 0);
        }

        close(fd);

        return;
    }

    return;
}
