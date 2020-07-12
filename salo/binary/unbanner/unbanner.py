from subprocess import call
from subprocess import Popen, PIPE
import pymysql
import requests
import sys
import time
import os


def netcryptm(data):
	child = Popen("../libcryptm/netcryptm --decrypt " + data, shell=True, stdin=PIPE, stdout=PIPE) 
	strings = list(child.stdout.read().split(b"\n"))
	return strings[0].decode("utf-8")



def main(usertoken, botname, dbname, dbserver, dbport, dbusername, dbpassword):
	#print(usertoken)
	#print(botname)

	if int(dbport) == 0:
		conn = pymysql.connect(host=dbserver, user=dbusername, passwd=dbpassword, db=dbname, unix_socket="/var/lib/mysql/mysql.sock")
	else:
		conn = pymysql.connect(host=dbserver, port=int(dbport), user=dbusername, passwd=dbpassword, db=dbname)
	
	cur = conn.cursor()
	
	apiurl = "https://api.vk.com/method/account.getBanned?v=5.60&offset=0&count=199&access_token=" + usertoken;

	response = requests.get(apiurl)
	rj = response.json()
	
	inbans = 0
	unbans = 0

	for user in rj["response"]["items"]:
		print((str(user["id"]) + " -- " + user["first_name"] + " " + user["last_name"] + " --"), end=" ")
		userid = str(user["id"])
		fname = user["first_name"]
		lname = user["last_name"]
		
		time.sleep(0.4)

		sql = "SELECT * FROM `"+dbname+"`.`ban_list` WHERE `id` = '"+botname+"_networking_vkcom_"+userid+"' LIMIT 1";
		cur.execute(sql)

		itr = 0

		for row in cur:
			itr = itr + 1

		if itr > 0:
			inbans = inbans + 1
			print("IN BAN LIST")
		else:
			unbans = unbans + 1
			print("NOT IN BAN LIST --", end=" ")
			apiurl1 = "https://api.vk.com/method/account.unbanUser?v=5.60&user_id="+userid+"&access_token="+usertoken;
			response1 = requests.get(apiurl1)
			print(response1.text)

	time.sleep(1)
	cur.close()
	conn.close()

	print("\n\n ----> unbanned " + str(unbans) + " accounts (" + str(inbans) + " in ban list), on bot " + botname + " <---- \n\n")





if __name__ == "__main__" :
	if (len(sys.argv) <= 1):
		print("Invalid cmdline args!!! sys.argv is less or equal than 1!!!")
		exit(-1)
	
	token = netcryptm(sys.argv[1])
	dbstr = netcryptm(sys.argv[2])
	qrstr = netcryptm(sys.argv[3])

	db = list(dbstr.split(";"))
	qr = list(qrstr.split(";"))

	main(token, qr[0], qr[1], db[0], db[1], db[2], db[3])