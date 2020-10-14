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

import sqlite3 as lite
import requests
import json
import time
import urllib.request
import os.path
from vklancer import api
from vklancer import utils


def userlogin(logina, passa):
	con = lite.connect('/home/montekekler/SCRSd/aucache.sqlite')
	cur = con.cursor() 

	if(logina[0] == '+'):
		reallogin = logina[1:]
	else:
		reallogin = logina

	cur.execute("SELECT token FROM accountsCache WHERE login=" + str(reallogin) + " AND expires>"+ str(int(time.time())) +" LIMIT 1")
	
	try:
		tok = cur.fetchall()[0][0]
	except IndexError:
		#логин и добавить токен
		print ("--------------> logging in again")
		tok = utils.oauth(logina, passa)
		expireTime = int(time.time()) + 43200

		cur.execute("DELETE FROM accountsCache WHERE login="+ str(reallogin))
		cur.execute("INSERT INTO accountsCache(login, token, expires) VALUES ("+ str(reallogin)+", \""+str(tok)+"\", "+str(expireTime)+")")
		con.commit()

		return tok;
	else: 
		print ("--------------> found saved token!")
		return tok;

	#tok = utils.oauth(logindata[0], logindata[1])
	exit()



def uploadOnVk(png_path, account):

	if(account[0] == 'g'):
		arr = account.split("zz")
		uid = "-" + arr[0][1:] + "_"
		token = arr[1]
		vk = api.API(token)
		resp = vk.photos.getMessagesUploadServer(group_id=int(arr[0][1:]))
	else:
		arr = account.split("zz")
		uid = arr[0][1:] + "_"
		token = arr[1]
		logindata = token.split("@")
		tokenreal = userlogin(logindata[0], logindata[1])
		vk = api.API(tokenreal)
		resp = vk.photos.getMessagesUploadServer(user_id=int(arr[0][1:]))



	#print(resp)

	link = resp.get('response').get('upload_url')

	print(link)

	multipart_form_data = {
	    'file': open(png_path, 'rb'),
	}

	response = requests.post(link, files=multipart_form_data)

	obj = json.loads(response.text)

	resp1 = vk.photos.saveMessagesPhoto(photo=obj["photo"], server=obj["server"], hash=obj["hash"])
	#print(resp1)

	entity = "photo" + uid + str(resp1.get('response')[0].get('id'))
	print(entity)
	return entity


def uploadGifOnVk(gif_path):
	vk = api.API('ffffffffffffffffffffffffffffffffffffffffffffffff')
	resp = vk.docs.getWallUploadServer(group_id=111111111111111111)

	link = resp.get('response').get('upload_url')

	print(link)

	multipart_form_data = {
	    'file': open(gif_path, 'rb'),
	}

	response = requests.post(link, files=multipart_form_data)

	obj = json.loads(response.text)

	resp1 = vk.docs.save(file=obj["file"], title='saloint.com')
	print(resp1)

	entity = "doc" + str(resp1.get('response')[0].get('owner_id')) + "_" + str(resp1.get('response')[0].get('id'))
	return entity


def downloadFromVk(jpg_path, photoname, account):
	if(photoname == "null"):
		return

	urllib.request.urlretrieve(photoname, jpg_path)

	if(os.path.isfile(jpg_path)):
		print ("SUCCESSFULLY DOWNLOADED! path = " + jpg_path)
		return
	else:
		print ("CANNOT DOWNLOAD FILE! url = " + photoname)
		exit()
