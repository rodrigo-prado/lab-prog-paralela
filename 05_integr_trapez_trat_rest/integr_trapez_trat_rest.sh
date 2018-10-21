#!/bin/bash
echo "Proccess execution"

# Building the launching command
echo "First execution!"
mpirun -np 12 integr_trapez_trat_rest.exe
#echo "Second execution"
#mpirun -np 9 integr_trapez_trat_rest.exe

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
