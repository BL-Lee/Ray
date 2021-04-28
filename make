#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd "${DIR}"

g++ -lpthread -mavx2 -O2 --std=c++11 src/ray.cpp -o ray
#./ray
#open out.bmp
