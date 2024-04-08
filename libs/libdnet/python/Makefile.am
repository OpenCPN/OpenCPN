## $Id$

include $(top_srcdir)/Makefile.am.common

if PYTHON
# XXX - nasty automake fudging here...

noinst_SCRIPTS = python-build


python-build: 
	$(PYTHON) setup.py build
	touch python-build

python-clean:
	rm -rf build dnet.c 
clean: python-clean

distclean: clean

python-install:
	$(PYTHON) setup.py install --prefix=$(DESTDIR)$(prefix)

install-exec-local: python-install

TESTS = test.py

else

install-exec-local:

endif

EXTRA_DIST = dnet.pyx dnet.c setup.py.in test.py interfacefinder.py

DISTCLEANFILES = *~
