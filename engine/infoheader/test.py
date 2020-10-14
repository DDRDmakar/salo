from subprocess import call
from subprocess import Popen, PIPE
import sys

cmdline = "/usr/bin/python3 infoheader.py"
Name = "testtask"
basepath = "/home/muxamed666/projects/PYTHON/SCRS/environment"

taskproc = Popen(cmdline + " " + Name + " " + basepath, shell=True)
#procdump = str(taskproc.stderr.read().decode(sys.stdout.encoding) + "\n" + taskproc.stdout.read().decode(sys.stdout.encoding))
