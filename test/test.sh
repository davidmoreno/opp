#!/bin/sh

set -e
BUILD="$( dirname $0 )/../build/"

if [ ! -e "$BUILD" ]; then
  mkdir -p $BUILD
  cd $BUILD
  conan install ..
  cmake ..
else
  cd $BUILD
fi

make -j8
# cat ../test/testfile.txt | timeout 5 bin/sbds
echo "test" | timeout 5 bin/test_tcp localhost 1234
