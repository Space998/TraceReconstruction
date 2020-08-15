nvcc -arch=sm_30 -dlink -o filelink.o $@ -L/usr/local/cuda-10.2/lib64 -lcudadevrt -lcudart
