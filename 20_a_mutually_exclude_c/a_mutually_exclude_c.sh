#!/bin/bash
echo "Proccess execution"

# Building the launching command
command="mpirun -np 6 a_mutually_exclude_c.exe"

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
