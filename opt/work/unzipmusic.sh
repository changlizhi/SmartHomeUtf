#!/bin/bash
# downfilelist.sh
zipFile="/tmp/mounts/SD-P1/music.zip"
mp3File="/tmp/mounts/SD-P1/play/shock.mp3"
playdir="/tmp/mounts/SD-P1/play"
cd /tmp/mounts/SD-P1/

if [ ! -d /tmp/mounts/SD-P1/play ]; then
 mkdir /tmp/mounts/SD-P1/play
fi
echo "123"
if [ -f $zipFile ]; then
	echo "try to unzip ... music.zip....."
	pwd=`uci -c/opt/ft get ftconfig.@ftconfig[0].musicpwd`
	echo "passwd:$pwd"
	if [ ! -f $mp3File ]; then
	 unzip -P $pwd -o /tmp/mounts/SD-P1/music.zip -d /tmp/mounts/SD-P1/play/
	 echo "unzip ...music.zip....finshed."
	fi
else
	aplay /tmp/mounts/SD-P1/voice/musicefileInvalid.wav
fi