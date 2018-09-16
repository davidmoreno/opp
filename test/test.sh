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

ninja || make -j8
case "$1" in
  "")
    cat ../test/testfile.txt | timeout 5 bin/sbds
    timeout 5 bin/startstop
    ;;
  "startstop")
    timeout 15 bin/startstop
    ;;
  "sbds")
    cat ../test/testfile.txt | timeout 5 bin/sbds
    ;;
esac

echo "---"
