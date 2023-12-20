#!/bin/sh
# Temporary support for 3513, to be removed
#
set -x

test -d  gui/src || mkdir -p gui/src
for f in $(tr ";" "\n" < build/gui-src.txt); do
  mv $f gui/src
done

test -d model/src || mkdir -p model/src
for f in $(tr ";" "\n" < build/model-src.txt); do
  mv $f model/src
done
