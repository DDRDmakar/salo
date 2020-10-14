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

from PIL import ImageFont, ImageDraw,  Image
import json
import sys
import uploaderh
import datetime

if (len(sys.argv) <= 1):
	print("Invalid cmdline args!!! sys.argv is less or equal than 1!!!")
	exit(-1)

basepath = sys.argv[2] + "/"
taskname = sys.argv[1]
taskpath = basepath + "exchange/" + taskname
enginepath = basepath + "engine/infoheader/"

taskstr = open(taskpath+"/task.txt").read()
task = json.loads(taskstr)


cfgtxt = open(enginepath+"resources/configuration.json").read()

cfgobject = json.loads(cfgtxt)

imagename = enginepath+"resources/"+cfgobject["image_filename"]
fontname = enginepath+"resources/"+cfgobject["font_filename"]
fontsize = cfgobject["textsize"]
color = (cfgobject["text_color_RGB"][0], cfgobject["text_color_RGB"][1], cfgobject["text_color_RGB"][2])
x = cfgobject["textblock_X"]
y = cfgobject["textblock_Y"]

img = Image.open(imagename)
draw = ImageDraw.Draw(img)
font = ImageFont.truetype(fontname, fontsize)

# "Сообщений за день: 97 274"
# "Сообщений за час: 5 533"
# "Уникальных пользователей: 1 004 404"
# "Аптайм: 1 день, 20 часов, 11 минут"
# "Все боты работают нормально"

draw.text((x, y+0), task["line1"], font=font, fill=color)
draw.text((x, y+30), task["line2"], font=font, fill=color)
draw.text((x, y+60), task["line3"], font=font, fill=color)
draw.text((x, y+90), task["line4"], font=font, fill=color)
draw.text((x, y+120), task["line5"], font=font, fill=color)

img.save(enginepath+"temp/scrs_out.png")
#print(img.info)

uploaderh.upload(enginepath+"temp/scrs_out.png", cfgobject["left_upper_X"], cfgobject["left_upper_Y"], cfgobject["right_lower_X"], cfgobject["right_lower_Y"])
