###
###
### SaloIntellect project
### Copyright (C) 2015-2020 Motylenok Mikhail, Makarevich Nikita
### 
### This program is free software: you can redistribute it and/or modify
### it under the terms of the GNU General Public License as published by
### the Free Software Foundation, either version 3 of the License, or
### (at your option) any later version.
### 
### This program is distributed in the hope that it will be useful,
### but WITHOUT ANY WARRANTY; without even the implied warranty of
### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
### GNU General Public License for more details.
### 
### You should have received a copy of the GNU General Public License
### along with this program.  If not, see <http://www.gnu.org/licenses/>.
###
###

from subprocess import call
from subprocess import Popen, PIPE
import uploader
import sys
import os

print("RUNNING SYNTH.PY ROUNTINES!")

if (len(sys.argv) <= 1):
	print("Invalid cmdline args!!! sys.argv is less or equal than 1!!!")
	exit(-1)

#print(os.getcwd())

basepath = sys.argv[2] + "/"
taskname = sys.argv[1]
taskpath = basepath + "exchange/" + taskname
enginepath = basepath + "engine/synth/"


fb = open(taskpath + "/task.txt", 'r')
speechtext = fb.read()
fb.close()

tts_engine = "./alex.sh"

if(speechtext[0] == 'b'):
	tts_engine = "./anna.sh"

speechtext = speechtext[1:]

#call TTS engine
taskproc = Popen(enginepath+tts_engine + " \"" + speechtext + "\" " + taskpath, shell=True, stdin=PIPE, stdout=PIPE)
procdump = str(taskproc.stdout.read().decode(sys.stdout.encoding))

print(procdump)

#call uploader
finallink = uploader.uploadOnVk(taskpath + "/out.ogg")

print(finallink)

ft = open(taskpath + "/output.txt", 'w')
ft.write(finallink)
ft.close()

print("SYNTH.PY OK, GOODBYE!")
