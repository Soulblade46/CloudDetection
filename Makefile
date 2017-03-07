CFLAGS=-std=c99 -Wall -Wextra -g -O3 -march=native $(shell pkg-config --cflags hdf5)
LDLIBS=-lrt -lOpenCL  $(shell pkg-config --libs hdf5)

lite:

