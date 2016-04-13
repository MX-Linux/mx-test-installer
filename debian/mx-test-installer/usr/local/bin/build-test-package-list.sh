#!/bin/bash

# get package list based on architecture

if [ -f ~/.config/mx-test-installer/packagelist.txt ]; then
	rm -f ~/.config/mx-test-installer/packagelist.txt
fi

if [ -d ~/.config/mx-test-installer ]; then
	echo "directory found"
	else
	mkdir -p ~/.config/mx-test-installer
fi

arch=$(dpkg --print-architecture)

if [ "$arch" = "i386" ]; then
	wget http://mxrepo.com/mx/testrepo/dists/mx15/test/binary-i386/Packages.gz -O ~/.config/mx-test-installer/Packages.gz
else
	wget http://mxrepo.com/mx/testrepo/dists/mx15/test/binary-amd64/Packages.gz -O ~/.config/mx-test-installer/Packages.gz
fi

#unpack Packages.gz
gzip -df ~/.config/mx-test-installer/Packages.gz 


#build short package description file

IFS=$'\n'

declare -a packagename
declare -a packageversion
declare -a packagedescrip

packagename=(`cat ~/.config/mx-test-installer/Packages |grep Package: |cut -d " " -f2`)
packageversion=(`cat ~/.config/mx-test-installer/Packages |grep -v Python-Version|grep Version: |cut -d " " -f2`)
packagedescrip=(`cat ~/.config/mx-test-installer/Packages |grep Description: |cut -d ":" -f2`)

count=$(echo ${#packagename[@]})
echo $count Packages

i="0"
while [ "$i" -lt "$count" ]
do
	echo "${packagename[i]} ${packageversion[i]} ${packagedescrip[i]}">>~/.config/mx-test-installer/packagelist.txt
	i=$[$i+1]
done

rm -f ~/.config/mx-test-installer/Packages.gz
rm -f ~/.config/mx-test-installer/Packages