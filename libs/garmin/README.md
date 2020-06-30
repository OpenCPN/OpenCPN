Garmin communication library
============================

The base of this library is the jeeps/ directory. This is an old, abandoned
upstream.

The current code is imported from gpsbabel, http://github.com/gpsbael/gpsbabel.
Gpsbabel han bundled the jeeps directory and also added som files with a
gb prefix

The inevitable changes required, mostly to get rid of Qt dependencies and
also to map logging to opencpn's functions are guarded by
LIBRARY\_BUILD #ifdefs.

Last import was done from the 03d8e1891 commit.

New files added when importing to opencpn includes the cmake files and the
opencpn/ directory

