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

# vklancer libs for VK API
from vklancer import api
from vklancer import utils
from datetime import datetime, date, time, timedelta
import random
import requests
import urllib.request
import os.path
import json
import time

#============================================================#
#                                                            #
#               MEMES SOURCES AND DESTINATIONS               #
#                                                            #
#============================================================#

dests = { 
        "SOURCE_1" : [ 
                { "name" : "Example1", "id" :  111111, "days_latnecy" : 1, "meme_per_day" : 1 },
                { "name" : "Example2", "id" :  222222, "days_latnecy" : 1, "meme_per_day" : 4 },
                { "name" : "Example3", "id" :  222222, "days_latnecy" : 1, "meme_per_day" : 4 }
            ]
        }
        

sources = { "SOURCE_1" : { "name" : "meme group example", "link" : "club<id here>", "id" : 111111, "count" : 86, "offset" : 16 } }

#============================================================#

buid = 111111111 
btok = "ffffffffffffffffffffffffffffffffffffffffffff";
bicepsVK = api.API(btok);
service_token = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
serviceVK = api.API(service_token)

# useful funcs

def downloadFromVk(jpg_path, photoname):
    if(photoname == "null"):
        return

    time.sleep(2)
    urllib.request.urlretrieve(photoname, jpg_path)

    if(os.path.isfile(jpg_path)):
        #print ("SUCCESSFULLY DOWNLOADED! path = " + jpg_path)
        return
    else:
        print ("CANNOT DOWNLOAD FILE! url = " + photoname)
        return

def selectBestSizeUrl(sizes_object):
    sizes = ["w", "z", "y", "r", "q", "p", "o", "x", "m", "s"];

    br = False;

    for sz in sizes:
        for obj in sizes_object:
            if obj["type"] == sz:
                return(obj["url"]);
                br = True;
                break;
        if br:
            break;

    return "null";


def uploadOnVk(jpg_path, token, uid):
    vk = api.API(token)
    resp = vk.photos.getWallUploadServer(user_id=uid)
    
    time.sleep(2)
    #print(resp)

    try:
        link = resp.get('response').get('upload_url')
    except AttributeError:
        return "null"

    #print(link)

    multipart_form_data = {
        'file': open(jpg_path, 'rb'),
    }

    response = requests.post(link, files=multipart_form_data)

    obj = json.loads(response.text)

    #print("---- photo = \n" + obj["photo"] + "----- hash" )

    #print("\n\n ---------- FILE ID ON VK ---------- \n\n")
    #print(obj["file"])
    #print("\n\n Result: \n")

    resp1 = vk.photos.saveWallPhoto(photo=obj["photo"], server=obj["server"], hash=obj["hash"])
    #print(resp1)

    try:
        entity = "photo" + str(uid) + "_" + str(resp1.get('response')[0].get('id'))
    except TypeError:
        return "null"
    
    #print(entity)
    return entity


# COLLECTING, CACHING & POSTING MEMES

for source in sources:
    destlist = dests[source]
    sourceparams = sources[source]
    selectedList = list();
    for dest in destlist:
        memecount = dest["meme_per_day"];
        memelist = serviceVK.wall.get(owner_id=-1*(sourceparams["id"]), count=sourceparams["count"], offset=sourceparams["offset"], filter="all", v=5.92);
        mememax = len(memelist["response"]["items"]);
        random.seed(version=2)

        destMemeIdList = list()

        while len(destMemeIdList) < memecount:
            candidateId = random.randint(1, mememax-1)
            if candidateId not in selectedList:
                destMemeIdList.append(candidateId)
                selectedList.append(candidateId)
            pass

        print(destMemeIdList)

        destMemeObjectsList = list()

        for memeId in destMemeIdList:
            if memelist["response"]["items"][memeId]["text"] == "":
                if memelist["response"]["items"][memeId]["attachments"][0]["type"] == "photo":
                    addr = selectBestSizeUrl(memelist["response"]["items"][memeId]["attachments"][0]["photo"]["sizes"])
                    destMemeObjectsList.append(addr)

        print(destMemeObjectsList)

        uploadedPhotoObjects = list()

        i = 0
        rnd = random.randint(100, 100500);
        for url in destMemeObjectsList:
            i = i + 1
            downloadFromVk("/home/montekekler/AMPS/cache/"+str(rnd)+"_"+str(i)+".jpg", url)
            upl = uploadOnVk("/home/montekekler/AMPS/cache/"+str(rnd)+"_"+str(i)+".jpg", btok, buid)
            if upl != "null":
                uploadedPhotoObjects.append(upl)
            os.remove("/home/montekekler/AMPS/cache/"+str(rnd)+"_"+str(i)+".jpg") 
        
        # POSTING MEMES
        random.seed(version=2)
        print(uploadedPhotoObjects)

        dt = datetime.now() + timedelta(days = dest["days_latnecy"])
        hour = 19
        minutes = random.randint(1, 59);
        dt = dt.replace(hour=hour, minute=minutes)

        for uploadedObject in uploadedPhotoObjects:
            print(dt, end="")
            untime = time.mktime(dt.timetuple())
            print(" --- UNIX:"+str(untime))
            res = bicepsVK.wall.post(owner_id=-1*(dest["id"]), attachments=uploadedObject, from_group=1, publish_date=int(untime), v=5.92)
            print(res)
            hour = hour-2
            minutes = random.randint(1, 59)
            dt = dt.replace(hour=hour, minute=minutes)
            time.sleep(2)


#print(resp)
