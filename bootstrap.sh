#!/bin/sh

if test -x "`which autoreconf`"; then
  AUTORECONF="autoreconf -i"
  exec $AUTORECONF
else
  echo "autoreconf does not exist in $PATH - unable to bootstrap. Feel free to try running"
  echo "./configure"
  exit 1
fi

exit 0
