#!/bin/bash

BUILD_TYPE="Debug"

if [[ "$1" == "--release" ]]; then
    BUILD_TYPE="Release"
fi

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..
cmake --build .
