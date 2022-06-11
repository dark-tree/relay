#!/bin/bash

set -x

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=release ..
make
