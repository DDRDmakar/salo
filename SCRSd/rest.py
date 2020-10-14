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

# Необходимые для работы модули
import http.server
import socketserver
import threading
import urllib.parse

# Модули SCRS
import job
import access


#
# Возвращает начальную страницу-заглушку
#

def getMotd():
	f = open('motd.txt', 'r')
	cont = f.read()
	f.close()
	return cont


#
# Обрабатывает аргументы в GET-запросе
#

def parse(str1):
	if(str1.find("&") == -1):
		return -1

	arrp = str1.split("&")
	
	t = arrp[0].find("/?task=")
	e = arrp[1].find("engine=")
	a = arrp[2].find("access=")
	#dat = arrp[3].find("multimedia=")

	if(t == -1 or e == -1 or a == -1):
		return "Error 1"
	if(t != 0 or e != 0 or a != 0):
		return "Error 2"

	t = arrp[0][7:]
	e = arrp[1][7:]
	a = arrp[2][7:]
	
	if(len(arrp) == 4):
		dat = arrp[3][11:]
	else:
		dat = ""

	return [t, e, a, dat]


#
# Обеспечивает авторизацию и доступ к задачам
#

def process(paramters):
	if(access.valid(paramters[2]) == 1):
		paramters[0] = urllib.parse.unquote(paramters[0])
		print(paramters)
		res = job.do(paramters[0], paramters[1], paramters[3])
		return res
	else:
		return "Error 3"


#
# Обрабатывает запросы по http 
#

class restHandler(http.server.BaseHTTPRequestHandler):
	def do_GET(this):
		this.protocol_version='HTTP/1.1'
		this.send_response(200, 'OK')
		this.send_header('Content-type', 'text/html')
		this.end_headers()

		if(this.path == "" or this.path == "/"):
			answer = getMotd() + this.version_string() + "; " + this.date_time_string() + "; " + this.address_string()
		else:
			params = parse(this.path)
			if(type(params) == str):
				answer = params
			elif(type(params) == list):
				answer = process(params)
			else:
				answer = "Error 4"

		this.wfile.write(bytes(answer, 'UTF-8'))


print("Running SCRS Server dispatcher @port=8000 ...")
port = 8000
Handler = restHandler
job.setTaskCounter(0)

httpd = socketserver.TCPServer(("", port), Handler)  
httpd.serve_forever()

print("SCRS Server terminated...")
