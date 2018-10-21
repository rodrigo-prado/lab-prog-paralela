#!/bin/bash
echo "Proccess execution"

# Building the launching command
command="/usr/lib64/openmpi/bin/mpirun -np 6 prop_sem_real.exe"

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
