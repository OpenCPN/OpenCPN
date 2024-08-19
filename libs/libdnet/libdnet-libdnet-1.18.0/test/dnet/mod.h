/*
 * mod.h
 *
 * Copyright (c) 2002 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#ifndef MOD_H
#define MOD_H

struct mod {
	char	*name;
	int	 type;
	int	(*main)(int argc, char *argv[]);
};

/*
 * Module types
 */
#define MOD_TYPE_DATA	0x01	/* generate data */
#define MOD_TYPE_ENCAP	0x02	/* encapsulate data */
#define MOD_TYPE_XMIT	0x04	/* send datagrams */
#define MOD_TYPE_KERN	0x08	/* kernel network info */

#endif /* MOD_H */
