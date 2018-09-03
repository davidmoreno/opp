#!/bin/sh

set -e
BUILD="$( dirname $0 )/../build/"

if [ ! -e "$BUILD" ]; then
  mkdir -p $BUILD
  cd $BUILD
  conan install ..
  cmake -G Ninja ..
else
  cd $BUILD
fi

ninja || make
cat ../test/testfile.txt | bin/sbds
