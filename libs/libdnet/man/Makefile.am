## $Id$

include $(top_srcdir)/Makefile.am.common

man_MANS = dnet.3

EXTRA_DIST = $(man_MANS)

dnet.3.txt: dnet.3
	groff -t -e -mdoc -Tascii dnet.3 | col -bx > $@

