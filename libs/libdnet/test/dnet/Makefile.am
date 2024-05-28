## $Id$

include $(top_srcdir)/Makefile.am.common

sbin_PROGRAMS = dnet

dnet_SOURCES = addr.c arp.c aton.c aton.h dnet.c eth.c fw.c hex.c \
	icmp.c intf.c ip.c mod.h rand.c route.c send.c tcp.c udp.c

man_MANS = dnet.8

EXTRA_DIST = $(man_MANS)

dnet_LDADD = $(top_builddir)/src/libdnet.la

dnet.8.txt: dnet.8
	groff -t -e -mdoc -Tascii dnet.8 | col -bx > $@

