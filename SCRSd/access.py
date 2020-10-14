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

from datetime import datetime
import hashlib

#
# Проверяет ключ на валидность
#

def valid(key):

	#return 1

	localkey = "{:%m***%d***%Y***scrskek^^^%H:::%H}".format(datetime.now())
	localkey = localkey.encode('utf-8')

	md5digL = hashlib.md5(localkey).hexdigest()
	sha1digR = hashlib.sha1(localkey).hexdigest()
	md5final = hashlib.md5((md5digL + "_" + sha1digR).encode('utf-8')).hexdigest()

	#print(md5final)

	if(md5final == key):
		return 1
	else:
		return 0
