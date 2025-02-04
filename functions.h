/****************************************
* CSE_130 Assignment_2
* functions.h
****************************************/

#pragma once

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

void read_request(int sd);

void status(int code, int sd, int needok, long int contlen);

void order_get(int sd, char *uri);

void order_put(int sd, char *uri, int contlen, char *cont);
