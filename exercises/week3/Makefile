.PHONY: clean

all: trapezoidal trapezoidal_v2 broadcast

trapezoidal: trapezoidal.cpp
	mpicxx $^ -O3 -std=c++11 -o $@

trapezoidal_v2: trapezoidal_v2.cpp
	mpicxx $^ -O3 -std=c++11 -o $@

broadcast: broadcast.cpp
	mpicxx $^ -O3 -std=c++11 -o $@

clean:
	rm trapezoidal trapezoidal_v2 broadcast
