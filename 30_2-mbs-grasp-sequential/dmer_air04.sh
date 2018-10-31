#!/bin/bash
echo "Proccess execution"

# Building the launching command
command="./dmer air04.mps.g.g"

$command
TWE=$?

if [ $TWE -eq 0 ]
then
  echo "End of process"
else
  echo "Process terminated with errors!"
fi
