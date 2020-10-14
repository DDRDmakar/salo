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
import random
import os
import shutil
import sys
import threading

#
# sets Task Counter
#

tasksCounter = 0
def setTaskCounter(num):
	global tasksCounter 
	tasksCounter = num
	return tasksCounter


#
# Возвращает вывод в случае ошибки
#

def jobErr(task):
	fb = open('basepath.txt', 'r')
	basepath = fb.read()
	fb.close()

	path = basepath + "/exchange/" + task + "/stdout.txt"

	try:
		fe = open(path, 'r')
		err = fe.read()
		fe.close()	
	except IOError:
		err = "STDOUT CONSOLE DUMP IS UNAVAILIBLE!"

	shutil.rmtree(basepath + "/exchange/" + task)

	return err


#
# возвращает езультат выполнения старой задачи
# разные движки требуют возвращать разные результаты 
#

def jobGet(task, engine):
	fb = open('basepath.txt', 'r')
	basepath = fb.read()
	fb.close()

	if(engine == "synth_get"):
		fullName = basepath + "/exchange/" + task + "/output.txt"
		
		try:
			fz = open(fullName, 'r')
			answ = fz.read()
			fz.close()		
		except IOError:
			return "Unresulted."

		shutil.rmtree(basepath + "/exchange/" + task)

		return answ

	if(engine == "krapiva_get"):
		fullName = basepath + "/exchange/" + task + "/output.txt"
		
		try:
			fz = open(fullName, 'r')
			answ = fz.read()
			fz.close()		
		except IOError:
			return "Unresulted."

		shutil.rmtree(basepath + "/exchange/" + task)

		return answ

	if(engine == "infoheader_get"):
		
		shutil.rmtree(basepath + "/exchange/" + task)

		return ""



#
# Подготовка к выполнению задачи
#

def preSetter(task, engine, base, data):
	if(engine == "synth_set"):
		#Предварительная подготовка не нужна
		return "/usr/bin/python3 "+base+"/engine/synth/synth.py"

	if(engine == "infoheader_set"):
		#Предварительная подготовка не нужна
		return "/usr/bin/python3 "+base+"/engine/infoheader/infoheader.py"

	if(engine == "krapiva_set"):
		
		child = Popen("libcryptm/netcryptm --decrypt " + data, shell=True, stdin=PIPE, stdout=PIPE) 
		strings = list(child.stdout.read().split(b"\n"))

		return "/usr/bin/python3 "+base+"/engine/krapiva/krapiva.py " + strings[0].decode("utf-8")
	return 0



#
# Функция асинхронно выполняет задачу
#

def jobSetWorker(task, engine, data, basepath, Name, fullNamed):
	#todo: uploader for pics!
	cmdline = preSetter(task, engine, basepath, data)

	#run task at different thread
	taskproc = Popen(cmdline + " " + Name + " " + basepath, shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
	procdump = str(taskproc.stderr.read().decode(sys.stdout.encoding) + "\n" + taskproc.stdout.read().decode(sys.stdout.encoding))

	errPath = fullNamed + "/stdout.txt"
	ef = open(errPath, 'w')
	ef.write(procdump)
	ef.close()


#
# Начинает новую задачу
#

def jobSet(task, engine, data):
	global tasksCounter
	Name = "task-" + str(tasksCounter) + "-" + str(random.randint(0, 99999))
	tasksCounter = tasksCounter + 1

	fb = open('basepath.txt', 'r')
	basepath = fb.read()
	fb.close()

	fullNamed = basepath + "/exchange/" + Name
	fullName = fullNamed + "/task.txt"

	os.makedirs(fullNamed)

	ft = open(fullName, 'w')
	ft.write(task)
	ft.close()

	####
	t = threading.Thread(target=jobSetWorker, args=(task, engine, data, basepath, Name, fullNamed))
	t.start()
	####

	print(fullName)
	print(fullNamed)

	return ("COMMAND SET OK: task id=\""+Name+"\"")

	

#
# Возвращает статус или создает новую задачу
#

def do(task, engine, data):
	mode = engine.split("_")

	if(type(mode) == str):
		return "Error 7"

	if(mode[1] == "set"):
		return jobSet(task, engine, data)
	elif(mode[1] == "get"):
		return jobGet(task, engine)
	elif(mode[1] == "err"):
		return jobErr(task)
	else:
		return "Error 5"

	
