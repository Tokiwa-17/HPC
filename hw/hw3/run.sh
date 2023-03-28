#! /bin/sh

set -x

make -j 4

THREAD_NUM=32

# OMP_NUM_THREADS=$THREAD_NUM srun -N 1 ./omp_sched

# OMP_NUM_THREADS=$THREAD_NUM OMP_SCHEDULE="dynamic" srun -N 1 ./omp_sched

# OMP_NUM_THREADS=$THREAD_NUM OMP_SCHEDULE="guided" srun -N 1 ./omp_sched
 
# srun -N 1 ./omp_sched > out/vanilla.txt

srun -N 1 ./omp_sched_static > out/static.txt

srun -N 1 ./omp_sched_dynamic > out/dynamic.txt

srun -N 1 ./omp_sched_guided > out/guided.txt