#!/bin/bash

echo "Building..."

declare -a laneWidths=("8" "4" "1")
for i in $laneWidths
do
    if ! g++ -DRAYS_PER_PIXEL=64 -DLANE_WIDTH=$i -lpthread -mavx2 -O2 --std=c++11 src/ray.cpp -o test/ray$i; then
	echo "There were errors in build for lane width of $i."
	exit 1
    fi
    echo "Lane width $i build successful"
done

if ! g++ -D__USE_OPENCL -DRAYS_PER_PIXEL=64 -framework OpenCL -O2 --std=c++11 src/rayGPU.cpp -o test/rayGPU; then
    echo "There were errors in build for GPU."
    exit 1
else
    echo "GPU ray build successful"
fi

echo "---------------\nBuild complete\n---------------"


test/ray8 test/ray8.bmp
echo "\n"
test/ray4 test/ray4.bmp
echo "\n"
test/ray1 test/ray1.bmp
echo "\n"
test/rayGPU test/rayGPU.bmp
echo "\nTests complete"


