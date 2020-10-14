#!/bin/bash

#echo $1 | RHVoice-test -p Aleksandr -o $2/out.wav && lame -V2 $2/out.wav $2/out.mp3
#echo $1 | /home/montekekler/RHVoice/RHVoice/build/linux/test/RHVoice-test -p Aleksandr -o $2/out.wav && oggenc $2/out.wav -o $2/out.ogg
echo $1 | RHVoice-client -s aleksandr+CLB >$2/out.wav && oggenc $2/out.wav -o $2/out.ogg
