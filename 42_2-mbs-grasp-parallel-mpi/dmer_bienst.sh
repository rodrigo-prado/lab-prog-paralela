#!/bin/bash
echo "Proccess execution"

# Building the launching command
#command="mpirun -np 4 dmer 1-t0.300-to0.05-y2003-m11-p90-signed.g.g"
command="/opt/openmpi-2.1.1/bin/mpirun -n 2 --machinefile my_hosts dmer bienst1.mps.g.g"
#command="mpirun -np 4 dmer 1-t0.300-to0.05-y2003-m11-p90-signed.g.g"


$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
