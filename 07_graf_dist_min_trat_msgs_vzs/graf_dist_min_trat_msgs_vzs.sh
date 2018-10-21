#!/bin/bash
echo "Proccess execution"

# Building the launching command
command="mpirun -np 6 graf_dist_min_trat_msgs_vzs.exe"

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
