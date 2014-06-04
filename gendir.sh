#!/bin/bash
#Junhong Chen jmj256 assignment 3 for cs720/820 fall 2013
#Oct 21, 2013

make clean >clean.log
make > make.log
uname > sys.log


fhd="test"
fmt=".in"
mkdir dir_1
cp .beautiful dir_1
for i in 1 2 3
do
    fname="$fhd$i$fmt"
    cp $fname dir_1
done
cd dir_1
mkdir A
mkdir nopermission
chmod ugo-r nopermission
mv test3.in A
cd ..

mkdir dir_2
for i in 1 2 3 4 5 6 7 8
do
    fname="$fhd$i$fmt"
    cp $fname dir_2
done
cd dir_2
mkdir A
mkdir B
mkdir C

k=0
for i in A B C
do 
    for j in 1 2
    do
	kk=$((k+j))
	fname="$fhd$kk$fmt"
	mv $fname $i
    done
    k=$((k+2))
done

cd A
mkdir F
cp test1.in F
cp test2.in F
cd F
mkdir G
mv test2.in G
cd ..
cd ..

bar=`pwd`
foo=`pwd`
pth="/D"
pp="$foo$pth"
ln -s $bar $pp

bar=`pwd`
foo=`pwd`
pth="/E"
pth1="/A"
pp="$foo$pth"
pp1="$bar$pth1"
ln -s $pp1 $pp

cd ..
foo=`pwd`
pth="/symlink"
pp="$foo$pth"
ln -s $pp1 $pp


cd dir_1
chmod ugo+r nopermission
cd ..