#!/bin/bash

DIR=/tmp/mx-test-repo-installer

# get package list based on architecture

if [ -f $DIR/packagelist.txt ]; then
	rm -f $DIR/packagelist.txt
fi

if [ -d $DIR ]; then
	echo "directory found"
	else
	mkdir -p $DIR
fi

#check mx16 repo

cat /etc/apt/sources.list.d/*.list |grep -q mx16
test=$?
echo "$test"
	
arch=$(dpkg --print-architecture)

if [ "$test" = "0" ]; then
	if [ "$arch" = "i386" ]; then
		wget http://mxrepo.com/mx/testrepo/dists/mx15/test/binary-i386/Packages.gz -O $DIR/mx15Packages.gz
		wget http://mxrepo.com/mx/testrepo/dists/mx16/test/binary-i386/Packages.gz -O $DIR/mx16Packages.gz
	else
		wget http://mxrepo.com/mx/testrepo/dists/mx15/test/binary-amd64/Packages.gz -O $DIR/mx15Packages.gz
		wget http://mxrepo.com/mx/testrepo/dists/mx16/test/binary-amd64/Packages.gz -O $DIR/mx16Packages.gz
	fi
	#unpack Packages.gz
	gzip -df $DIR/mx15Packages.gz
	gzip -df $DIR/mx16Packages.gz 

else
	if [ "$arch" = "i386" ]; then
		wget http://mxrepo.com/mx/testrepo/dists/mx15/test/binary-i386/Packages.gz -O $DIR/mx15Packages.gz
	else
		wget http://mxrepo.com/mx/testrepo/dists/mx15/test/binary-amd64/Packages.gz -O $DIR/mx15Packages.gz
	fi
	#unpack Packages.gz
	gzip -df $DIR/mx15Packages.gz 
fi


#build short package description file

IFS=$'\n'

declare -a packagename
declare -a packageversion
declare -a packagedescrip

packagename=(`cat $DIR/*Packages |awk '/Package:/ && !/-Package/'|cut -d " " -f2`)
packageversion=(`cat $DIR/*Packages |awk '/Version:/ && !/-Version:/'|cut -d " " -f2`)
packagedescrip=(`cat $DIR/*Packages |awk '/Description:/ && !/-Description:/'|cut -d ":" -f2`)

count=$(echo ${#packagename[@]})
echo $count Packages

i="0"
while [ "$i" -lt "$count" ]
do
	echo "${packagename[i]} ${packageversion[i]} ${packagedescrip[i]}">>$DIR/packagelist.txt
	i=$[$i+1]
done

rm -f $DIR/*Packages.gz
rm -f $DIR/*Packages
