#!/bin/bash
echo "Proccess execution"

# Building the launching command
command="mpirun -mca btl ^openib -np 1 dmer 1-t0.300-to0.05-y2003-m11-p90-signed.g.g"

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
