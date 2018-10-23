#!/bin/bash
echo "Proccess execution"

# Building the launching command
command="mpirun -mca btl ^openib -np 4 dmer 1-t0.300-to0.05-y2003-m11-p90-signed.g.g 1000 300"

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
