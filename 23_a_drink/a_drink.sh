#!/bin/bash
echo "Proccess execution"

# Building the launching command
command="mpirun -np 5 a_drink.exe"

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
