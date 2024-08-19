## $Id$

include $(top_srcdir)/Makefile.am.common

AM_CPPFLAGS += @CHECKINC@

if HAVE_CHECK
TESTS = check_addr check_arp check_blob check_eth check_fw check_ip \
	check_intf check_rand check_route
else
TESTS =
endif

noinst_PROGRAMS = $(TESTS)

check_addr_SOURCES = check_addr.c
check_arp_SOURCES = check_arp.c
check_blob_SOURCES = check_blob.c
check_eth_SOURCES = check_eth.c
check_fw_SOURCES = check_fw.c
check_intf_SOURCES = check_intf.c
check_ip_SOURCES = check_ip.c
check_rand_SOURCES = check_rand.c
check_route_SOURCES = check_route.c

LDADD = $(top_builddir)/src/libdnet.la @CHECKLIB@
