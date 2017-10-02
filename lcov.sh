#!/bin/sh

if [ -d ./lcov ]; then
  rm -Rf ./lcov
fi

if [ -e ./lcov.dat ]; then
  rm ./lcov.dat
fi

if [ -e ./lcov.tgz ]; then
  rm ./lcov.tgz
fi

lcov --capture --directory src --output-file ./lcov.dat
genhtml ./lcov.dat --output-directory lcov
tar -zcvf lcov.tgz lcov

