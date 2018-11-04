
import argparse
import time
import numpy
import multiprocessing
import subprocess
import shlex
import re


output = "96,2.96756,96,0.211584"
# output = "2,2.96756,2,2.96756"
m = re.search("^[0-9]*,([0-9]*[.][0-9]*),[0-9]*,([0-9]*[.][0-9]*)$", output, re.M)
print m.group(0)
print m.group(1)
print m.group(2)
