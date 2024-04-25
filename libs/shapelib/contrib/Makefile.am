CONTRIB_CFLAGS = -I$(top_srcdir) -DDEBUG -DDEBUG2

# Extra files to distribute in the source tarball
EXTRA_DIST = makefile.vc tests/shpproj.sh doc/Shape_PointInPoly_README.txt doc/shpsort.txt ShapeFileII.pas

# Installed executables
bin_PROGRAMS = csv2shp dbfcat dbfinfo shpcat shpdxf shpfix shpsort Shape_PointInPoly shpcentrd shpdata shpinfo shpwkb

csv2shp_SOURCES = csv2shp.c
csv2shp_CPPFLAGS = $(CONTRIB_CFLAGS)
csv2shp_LDADD = $(top_builddir)/libshp.la

dbfcat_SOURCES = dbfcat.c
dbfcat_CPPFLAGS = $(CONTRIB_CFLAGS)
dbfcat_LDADD = $(top_builddir)/libshp.la

dbfinfo_SOURCES = dbfinfo.c
dbfinfo_CPPFLAGS = $(CONTRIB_CFLAGS)
dbfinfo_LDADD = $(top_builddir)/libshp.la

shpcat_SOURCES = shpcat.c
shpcat_CPPFLAGS = $(CONTRIB_CFLAGS)
shpcat_LDADD = $(top_builddir)/libshp.la

shpdxf_SOURCES = shpdxf.c
shpdxf_CPPFLAGS = $(CONTRIB_CFLAGS)
shpdxf_LDADD = $(top_builddir)/libshp.la

shpfix_SOURCES = shpfix.c
shpfix_CPPFLAGS = $(CONTRIB_CFLAGS)
shpfix_LDADD = $(top_builddir)/libshp.la

shpsort_SOURCES = shpsort.c
shpsort_CPPFLAGS = $(CONTRIB_CFLAGS)
shpsort_LDADD = $(top_builddir)/libshp.la -lm

Shape_PointInPoly_SOURCES = Shape_PointInPoly.cpp
Shape_PointInPoly_CPPFLAGS = $(CONTRIB_CFLAGS)
Shape_PointInPoly_LDADD = $(top_builddir)/libshp.la

shpcentrd_SOURCES = shpcentrd.c shpgeo.c shpgeo.h
shpcentrd_CPPFLAGS = $(CONTRIB_CFLAGS)
shpcentrd_LDADD = $(top_builddir)/libshp.la -lm

shpdata_SOURCES = shpdata.c shpgeo.c shpgeo.h
shpdata_CPPFLAGS = $(CONTRIB_CFLAGS)
shpdata_LDADD = $(top_builddir)/libshp.la -lm

shpinfo_SOURCES = shpinfo.c shpgeo.c shpgeo.h
shpinfo_CPPFLAGS = $(CONTRIB_CFLAGS)
shpinfo_LDADD = $(top_builddir)/libshp.la -lm

shpwkb_SOURCES = shpwkb.c shpgeo.c shpgeo.h
shpwkb_CPPFLAGS = $(CONTRIB_CFLAGS)
shpwkb_LDADD = $(top_builddir)/libshp.la -lm

# Tests
TESTS_ENVIRONMENT = top_builddir=$(abs_top_builddir)
TESTS = tests/shpproj.sh
