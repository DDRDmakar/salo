from datetime import datetime
import hashlib

#
# Генерирует клиентский ключ
#

localkey = "{:%m***%d***%Y***scrskek^^^%H:::%H}".format(datetime.now())
localkey = localkey.encode('utf-8')
md5digL = hashlib.md5(localkey).hexdigest()
sha1digR = hashlib.sha1(localkey).hexdigest()
md5final = hashlib.md5((md5digL + "_" + sha1digR).encode('utf-8')).hexdigest()

print(md5final)
