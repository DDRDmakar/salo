import requests
import json
from vklancer import api

def uploadOnVk(mp3_path):
	vk = api.API('ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff')
	resp = vk.docs.getWallUploadServer(group_id=1111111111111111111, type='audio_message')

	link = resp.get('response').get('upload_url')

	print(link)

	multipart_form_data = {
	    'file': open(mp3_path, 'rb'),
	}

	response = requests.post(link, files=multipart_form_data)

	obj = json.loads(response.text)

	#print("\n\n ---------- FILE ID ON VK ---------- \n\n")
	#print(obj["file"])
	print("\n\n Result: \n")

	resp1 = vk.docs.save(file=obj["file"], title='saloint.com')
	print(resp1)

	entity = "doc" + str(resp1.get('response')[0].get('owner_id')) + "_" + str(resp1.get('response')[0].get('id'))
	return entity
