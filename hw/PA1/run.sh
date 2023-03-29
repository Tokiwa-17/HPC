#!/bin/bash

set -x

source /home/spack/spack/share/spack/setup-env.sh

spack load openmpi

make -j 4

elements_num=10000
filename=test

./generate $elements_num $filename.dat
# run on 1 machine * 28 process, feel free to change it!
srun -N 1 -n 20 ./odd_even_sort $elements_num ./$filename.dat

