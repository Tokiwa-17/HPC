#! /bin/bash

set -x

source /home/spack/spack/share/spack/setup-env.sh

spack load openmpi

make -j 1

# srun -N 1 -n 4 ./allreduce 10 100000000
srun -N 2 -n 4 ./allreduce 10 100000000