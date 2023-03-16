.PHONY: clean

all: trapezoidal

trapezoidal: trapezoidal.cpp
	mpicxx $^ -O3 -std=c++11 -o $@

clean:
	rm trapezoidal
