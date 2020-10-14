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

import requests
import json
from vklancer import api
from vklancer import utils

def upload(path, x, y, x2, y2):
	vk = api.API('ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff')
	resp = vk.photos.getOwnerCoverPhotoUploadServer(group_id = 11111111111, crop_x = x, crop_y = y, crop_x2 = x2, crop_y2 = y2)
	link = resp.get('response').get('upload_url')

	multipart_form_data = {
	    'file': open(path, 'rb'),
	}

	response = requests.post(link, files=multipart_form_data)

	obj = json.loads(response.text)

	resp1 = vk.photos.saveOwnerCoverPhoto(photo = obj["photo"], hash = obj["hash"])

	#print(resp1)
