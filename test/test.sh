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
ninja || make -j8
case "$1" in
  "")
    cat ../test/testfile.txt | timeout 5 bin/sbds
    timeout 5 bin/startstop
    echo "test" | timeout 5 bin/test_tcp localhost 1234
    ;;
  "startstop")
    timeout 15 bin/startstop
    ;;
  "sbds")
    cat ../test/testfile.txt | timeout 5 bin/sbds
    ;;
  "tcp")
    echo "test" | timeout 5 bin/test_tcp localhost 1234
    ;;
esac

echo "---"
