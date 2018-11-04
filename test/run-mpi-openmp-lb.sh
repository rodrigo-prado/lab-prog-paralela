#!/bin/bash

if [ -z "$1" ] ; then
  file=./input/danoint.mps.g.g
else
  file=$1
fi
if [ -z "$2" ] ; then
  maxiter=100
else
  maxiter=$2
fi
if [ -z "$3" ] ; then
  maxtime=30000
else
  maxtime=$3
fi
if [ -z "$4" ] ; then
  target=96
else
  target=$4
fi

# /opt/openmpi-2.1.1/bin/mpirun
prog="mpirun --machinefile host_proc+thread -mca btl ^openib -np 2 ./bin/dmer_MPI+OpenMP_LB"
# prog="/opt/openmpi-2.1.1/bin/mpirun --machinefile host_proc+thread -mca btl ^openib -np 2 ./bin/dmer_MPI+OpenMP_LB"

$prog $file $maxiter $maxtime $target
