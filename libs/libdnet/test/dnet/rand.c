/*
 * rand.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"

#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dnet.h"
#include "mod.h"

void
rand_usage(void)
{
	fprintf(stderr, "Usage: dnet rand <len>\n");
	exit(1);
}

int
rand_main(int argc, char *argv[])
{
	rand_t *r;
	int len;
	u_char *p;
	
	if (argc != 2 || *(argv[1]) == '-')
		rand_usage();
	
	if ((len = atoi(argv[1])) == 0)
		rand_usage();

	if ((p = malloc(len)) == NULL)
		err(1, "malloc");

	if ((r = rand_open()) == NULL)
		err(1, "rand_init");

	if (rand_get(r, p, len) < 0)
		err(1, "rand_get");
	
	if (write(STDOUT_FILENO, p, len) != len)
		err(1, "write");
	
	return (0);
}

struct mod mod_rand = {
	"rand",
	MOD_TYPE_DATA,
	rand_main
};
