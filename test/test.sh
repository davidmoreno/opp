#!/bin/sh

set -e
BUILD="$( dirname $0 )/../build/"

if [ ! -e "$BUILD" ]; then
  mkdir -p $BUILD
  cd $BUILD
  conan install ..
  # hack for ubuntu 18.04 // 2018-08-20
  sed -i 's/7.2/7.3/g' *.txt
  cmake ..
else
  cd $BUILD
fi

make
cat ../test/testfile.txt | bin/sbds
