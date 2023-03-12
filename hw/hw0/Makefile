.PHONY: clean

all: openmp_pow mpi_pow hello

openmp_pow: openmp_pow.cpp
	g++ $^ -O3 -std=c++11 -fopenmp -o $@

mpi_pow: mpi_pow.cpp
	mpicxx $^ -O3 -std=c++11 -o $@

hello: hello.cpp
	g++ $^ -O3 -std=c++11 -fopenmp -o $@

clean:
	rm mpi_pow openmp_pow
