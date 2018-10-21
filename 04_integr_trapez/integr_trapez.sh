#!/bin/bash
echo "Proccess execution"

# Building the launching command
echo "First execution!"
mpirun -np 7 integr_trapez.exe
echo "Second execution"
mpirun -np 8 integr_trapez.exe

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
