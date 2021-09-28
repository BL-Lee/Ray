all:
	g++ -Iassets/sequences -lpthread -mavx2 -O2 --std=c++11 src/ray.cpp -o ray
gpu:
	g++ -D__USE_OPENCL -O2 -framework OpenCL --std=c++11 src/rayGPU.cpp  -o ray
