#!/bin/sh
# Temporary support for 3513, to be removed
#
set -x

test -d  gui/include/gui || mkdir -p gui/include/gui
for f in $(tr ";" "\n" < build/gui-headers.txt); do
  mv $f gui/include/gui
done

test -d model/include/model || mkdir -p model/include/model
for f in $(tr ";" "\n" < build/model-hdrs.txt); do
  mv $f model/include/model
done
