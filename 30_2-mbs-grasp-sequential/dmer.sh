#!/bin/bash
echo "Proccess execution"

# Building the launching command
#command="./dmer danoint.mps.g.g 100 30000 96"
command="./dmer danoint.mps.g.g 100 30000"

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
