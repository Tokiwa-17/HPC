.PHONY: clean

all: omp_sched omp_sched_static omp_sched_dynamic omp_sched_guided

omp_sched: omp_sched.cpp
	g++ omp_sched.cpp -o omp_sched -O2 -fopenmp

omp_sched_static: omp_sched_static.cpp
	g++ omp_sched_static.cpp -o omp_sched_static -O2 -fopenmp

omp_sched_dynamic: omp_sched_dynamic.cpp
	g++ omp_sched_dynamic.cpp -o omp_sched_dynamic -O2 -fopenmp

omp_sched_guided: omp_sched_guided.cpp
	g++ omp_sched_guided.cpp -o omp_sched_guided -O2 -fopenmp

clean:
	rm omp_sched_static omp_sched_dynamic omp_sched_guided omp_sched
