#!/bin/sh

mkdir -p build
mkdir -p bin

cd build || { echo "cd build failure"; exit 1; }

cmake -D CMAKE_BUILD_TYPE=Debug CMAKE_INSTALL_PREFIX=. ..

make