#!/bin/sh

PREFIX=/usr/local
TERM_CODE=CP932
PROC_CODE_LIST='EUC-JP UTF-8'

set -e

if [ ! -d bin ]; then
  mkdir bin
fi

for proc_code in $PROC_CODE_LIST; do
  echo "** Build by $TERM_CODE / $proc_code"
  builddir=build-$proc_code
  rm -rf $builddir
  meson setup $builddir \
    --prefix=$PREFIX \
    --buildtype=release \
    -Ddefault_term_code=$TERM_CODE \
    -Ddefault_proc_code=$proc_code
  meson compile -C $builddir
  strip -x $builddir/cocot.exe
  cp -v $builddir/cocot.exe bin/cocot-$proc_code.exe
  rm -rf $builddir
done
