#! /bin/bash
set -e

source /home/spack/spack/share/spack/setup-env.sh

spack load openmpi

make -j 2

OMP_NUM_THREADS=2 srun -N 1 ./hello 
OMP_NUM_THREADS=3 srun -N 1 ./hello


echo "ALL DONE!"