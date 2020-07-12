#!/usr/bin/python3

from subprocess import call
from random import randint
import datetime
import sys
import os
import time


def timemarkDump(fname):
	fl = open(fname, "a")
	fl.write("\n------------------\n")
	fl.write("\nProcess death registered by ./watchdog.py on "+datetime.datetime.now().strftime("%d-%m-%Y %H:%M:%S")+"\n")
	fl.close()

def instancenate():
	call("pkill salobin", shell=True) 														#kontrolny vistrel
	filename = "crashdumps/dump-"+datetime.datetime.now().strftime("%d-%m-%Y_%H:%M:%S")		#crashdump filename
	salo_pid = randint(0, 99999)															#generate pid
	
	#run salo
	call("{ stdbuf -o 0 ./salobin -pid "+str(salo_pid)+"; } 2>&1 | tee \""+filename+"\"", shell=True)
	
	#exists
	if (not os.path.isfile("resources/watchdog.pid")):
		timemarkDump(filename)
		return -1 #file not exists => salo crashed

	#read pid
	saved_pid = int(open('resources/watchdog.pid', 'r').read())
	
	if(saved_pid == salo_pid):
		os.remove(filename)
		return 0 #pid is correct, all ok
	else:
		timemarkDump(filename)
		return -1 #pid not ours => salo crashed

	timemarkDump(filename)
	return -1


def main():
	ret = instancenate()

	#print(ret)
	#sys.exit()

	while ret != 0:
		tstart = time.time()
		ret = instancenate()
		tend = time.time()
		if(round(tend - tstart, 2) < 180.0):
			if ret != 0:
				print("Exited due to looping-protection!")
			sys.exit()
		pass

main()