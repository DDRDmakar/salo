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
import uploaderk
import sys
import os

print("RUNNING KRAPIVA.PY ROUNTINES!")

if (len(sys.argv) <= 1):
	print("Invalid cmdline args!!! sys.argv is less or equal than 1!!!")
	exit(-1)

#print(os.getcwd())

inputphoto = sys.argv[1]
accountkey = sys.argv[2]

basepath = sys.argv[4] + "/"
taskname = sys.argv[3]
taskpath = basepath + "exchange/" + taskname
enginepath = basepath + "engine/krapiva/"

engine = enginepath + "krapiva"

#call downloader
#TODO:
uploaderk.downloadFromVk(taskpath + "/input.jpg", inputphoto, accountkey)

#call krapiva
call([engine + " " + taskpath], shell=True)

#if it a gif
if (os.path.isfile(taskpath + "/output.gif")):
	finallink = uploaderk.uploadGifOnVk(taskpath + "/output.gif")
else: #if it a png
	finallink = uploaderk.uploadOnVk(taskpath + "/output.png", accountkey) #for png
	i = 1
	while i < 10:
		if (os.path.isfile(taskpath + "/output"+str(i)+".png")):
			finallink = finallink + "," + uploaderk.uploadOnVk(taskpath + "/output"+str(i)+".png", accountkey)
		i = i + 1
		pass

print(finallink)

ft = open(taskpath + "/output.txt", 'w')
ft.write(finallink)
ft.close()

print("KRAPIVA.PY OK, GOODBYE!")
