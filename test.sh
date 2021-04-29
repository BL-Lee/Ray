#!/bin/bash

echo "Building..."


if ! g++ -DRAYS_PER_PIXEL=64 -DLANE_WIDTH=8 -lpthread -mavx2 -O2 --std=c++11 src/ray.cpp -o test/ray8; then
    echo "There were errors in build for lane width of 8."
    exit 1
fi


if ! g++ -DRAYS_PER_PIXEL=64 -DLANE_WIDTH=4 -lpthread -O2 --std=c++11 src/ray.cpp -o test/ray4; then
    echo "There were errors in build for lane width of 4."
    exit 1
fi

compileStatus=
if ! g++ -DRAYS_PER_PIXEL=64 -DLANE_WIDTH=1 -lpthread -O2 --std=c++11 src/ray.cpp -o test/ray1; then
    echo "There were errors in build for lane with of 1."
    exit 1
fi
echo "---------------\nBuild complete\n---------------"

cd test
./ray8 ray8.bmp
echo "\n"
./ray4 ray4.bmp
echo "\n"
./ray1 ray1.bmp
echo "\nTests complete"


#open out.bmp
