#!/bin/bash

DIR=/tmp/mx-test-repo-installer

# get package list based on architecture

if [ -f /tmp/mx-test-repo-installer/packagelist.txt ]; then
	rm -f $DIR/packagelist.txt
fi

if [ -d $DIR ]; then
	echo "directory found"
	else
	mkdir -p $DIR
fi

arch=$(dpkg --print-architecture)

if [ "$arch" = "i386" ]; then
	wget http://mxrepo.com/mx/testrepo/dists/mx15/test/binary-i386/Packages.gz -O $DIR/Packages.gz
else
	wget http://mxrepo.com/mx/testrepo/dists/mx15/test/binary-amd64/Packages.gz -O $DIR/Packages.gz
fi

#unpack Packages.gz
gzip -df $DIR/Packages.gz 


#build short package description file

IFS=$'\n'

declare -a packagename
declare -a packageversion
declare -a packagedescrip

packagename=(`cat $DIR/Packages |grep Package: |cut -d " " -f2`)
packageversion=(`cat $DIR/Packages |grep -v Python-Version|grep -v Gstreamer-Version|grep Version: |cut -d " " -f2`)
packagedescrip=(`cat $DIR/Packages |grep Description: |cut -d ":" -f2`)

count=$(echo ${#packagename[@]})
echo $count Packages

i="0"
while [ "$i" -lt "$count" ]
do
	echo "${packagename[i]} ${packageversion[i]} ${packagedescrip[i]}">>$DIR/packagelist.txt
	i=$[$i+1]
done

rm -f $DIR/Packages.gz
rm -f $DIR/Packages
