#!/bin/bash

BUILDDT=`date +%d%m%y%H`
echo "Compilation id = 2$BUILDDT"

FILES="../*.cpp ../vk/*.cpp ../telegram/*.cpp ../web/*.cpp ../conveer/*.cpp"
#OPTIONS=" -pipe -mfpmath=sse -Ofast!!! -flto!!! -march=native -funroll-loops -std=c++11"
OPTIONS=" -ggdb -pipe -mfpmath=sse -Ofast -march=native -funroll-loops -std=c++11"
LIBS=" -lcryptm -lpthread -lcurl -lyaml-cpp -L ../libcryptm -I /usr/include/mysql `mysql_config --libs` -lgmpxx -lgmp -I /usr/local/include/ -L /usr/local/lib/libyaml-cpp.a"
BUILDID=" -D buildid='\"oldstyle-build\"'"

GCCLINE="g++ $FILES$OPTIONS$LIBS$BUILDID -o salobin"

echo -n "Compilation time (sec): "

/usr/bin/time -f%e /bin/sh -c "$GCCLINE"
strip salobin
file salobin
echo " "
echo " "
