#!/bin/bash


DEBUG=$1
DEBUG_AC=$2
DEBUG_SD=$3
DEBUG_KWS=$4

if [ -z "$DEBUG" ]; then
    echo "Build without debug flag"
    echo "C_FLAGS=\""
    echo "CXX_FLAGS=\""
    C_FLAGS=""
    CXX_FLAGS=""
else
    echo "Build with debug flag: $DEBUG"
    C_FLAGS="-DDEBUG=$DEBUG"
    CXX_FLAGS="-DDEBUG=$DEBUG"
    echo "C_FLAGS=${C_FLAGS}"
    echo "CXX_FLAGS=${CXX_FLAGS}"
fi

# DEBUG_AC not equel to 0
if [ ! "$DEBUG_AC" -eq 0 ]; then
    echo "Adding flag DEBUG_AC"
    C_FLAGS="${C_FLAGS} -DDEBUG_AC=$DEBUG_AC"
    CXX_FLAGS="${CXX_FLAGS} -DDEBUG_AC=$DEBUG_AC"
fi

if [ ! "$DEBUG_SD" -eq 0 ]; then
    echo "Adding flag DEBUG_SD"
    C_FLAGS="${C_FLAGS} -DDEBUG_SD=$DEBUG_SD"
    CXX_FLAGS="${CXX_FLAGS} -DDEBUG_SD=$DEBUG_SD"
fi

if [ ! "$DEBUG_KWS" -eq 0 ]; then
    echo "Adding flag DEBUG_KWS"
    C_FLAGS="${C_FLAGS} -DDEBUG_KWS=$DEBUG_KWS"
    CXX_FLAGS="${CXX_FLAGS} -DDEBUG_KWS=$DEBUG_KWS"
fi

# if no .cpp files in dictory and sub dictories
if [ -z "$(find . -name '*.cpp')" ]; then
    make clean
    make
else
    set -x
    # if build directory existed remove it
    rm -rf build
    mkdir -p build
    pushd build
    cmake -DCMAKE_C_FLAGS="${C_FLAGS}" -DCMAKE_CXX_FLAGS="${CXX_FLAGS}" ..
    make
    popd
fi
