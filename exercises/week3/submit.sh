#!/bin/bash
set -e

source /home/spack/spack/share/spack/setup-env.sh

spack load openmpi

make -j 1

# srun -N 1 -n 8 --cpu-bind sockets ./trapezoidal
# srun -N 1 -n 8 --cpu-bind sockets ./trapezoidal_v2
srun -N 1 -n 8 --cpu-bind sockets ./broadcast

echo "All Done!"