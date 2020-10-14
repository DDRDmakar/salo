#!/bin/bash

#BUILDDT=`date +%d%m%y%H`
#echo "Compilation id = 2$BUILDDT"

FILES="*.cpp"

#OPTIONS=" -pipe -mfpmath=sse -Ofast -march=native -funroll-loops -std=c++11"

# -lyaml-cpp -I /usr/local/include/ -L /usr/local/lib/libyaml-cpp.a

LIBS=" -I/usr/local/include/ImageMagick `Magick++-config --cxxflags --cppflags` `Magick++-config --ldflags --libs` `pkg-config --cflags --libs opencv4` -lyaml-cpp -I /usr/local/include/ -L /usr/local/lib/libyaml-cpp.a"

GCCLINE="g++ $FILES$LIBS -o krapiva"

$GCCLINE

#echo -n "Compilation time (sec): "

#/bin/sh -c "$GCCLINE"
