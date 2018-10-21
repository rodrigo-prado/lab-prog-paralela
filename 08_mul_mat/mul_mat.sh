#!/bin/bash
echo "Proccess execution"

# Building the launching command
command="mpirun -np 12 mul_mat.exe"

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
