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



echo
echo "-----------------------Compile Info-----------------------"
cat make.log
rm make.log
echo "--------------------------OS Info-------------------------"
echo "OS Info:" `cat sys.log`
rm sys.log
echo "----------------------------------------------------------"
echo
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++++++++  Correction Test 1 +++++++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo 
echo "Test 1.1"
echo "***** should print help information and return status 0"
echo "***** ./plcs -apbehq -t"
`./plcs -apbehq-t >output.log`
EXIT=$?
cat output.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
rm output.log



echo
echo
echo "Test 1.2"
echo "***** should print help information and return status 0"
echo "***** ./plcs -h <null ****"
`./plcs -h <null >output.log 2`
EXIT=$?
cat output.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
rm output.log




echo
echo
echo "Test 1.3"
echo "***** should print 'Missing search string' and return status 1"
echo "***** ./plcs <null ****"
`./plcs <null >output.log 2>err.log`
EXIT=$?
cat err.log
echo "status $EXIT"
echo "@@@@@ expected status 1, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log




echo
echo
echo "Test 1.4"
echo "***** should print 1 'Illegal switch', 4 'Illegal value for switch', 1 help information and return status 1"
echo "***** ./plcs -abc -l10000 -m0 -n0 -t-1 -h -d-1 <null ****"
`./plcs -abc -l10000 -m0 -n0 -t-1 -h -d-1<null >output.log`
EXIT=$?
cat output.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"

rm output.log



echo
echo
echo "Test 1.5"
echo "***** should print 'No such file' with realpath and return status 0"
echo "***** ./plcs hello no-such-file <null ****"
`./plcs hello no-such-file <null >output.log 2>err.log`
EXIT=$?
cat err.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log



echo
echo
echo "Test 1.6: correction for -i"
echo "***** should print 2 lines containing 'Life' or 'life' with realpath and return status 0"
echo "***** ./plcs -i LIFE tea banister<null ****"
`./plcs -i LIFE tea banister <null >output.log 2>err.log`
cat output.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log




echo
echo
echo "Test 1.7: correction"
echo "***** should print at most 4 lines from each file with realpath and nos and return status 0"
echo "***** ./plcs -bpv -m4 -n2 The old tea devil whole abc stopping quick<null ****"
`./plcs -bpv -m4 -n2 The old tea devil whole abc stopping quick <null >output.log 2>err.log`
cat output.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log




echo
echo
echo "Test 1.8: correction"
echo "***** should print 26 empty lines with with realpath and nos and return status 0"
echo "***** ./plcs -pbe -n5 -l1 "" road<null ****"
`./plcs -pbe -n5 -l1 "" road <null >output.log 2>err.log`
echo "OUPUT:"
cat output.log
echo "ERROR MESSAGE:"
cat err.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log


echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "++++++++++++++++++++++++++++  Dir Test 2 +++++++++++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo
echo
echo "---------------------------- Test 2.1: option -a ---------------------"
echo "***** should print 8 lines with realpath, 1 error message, and return status 0. File name starting with . is not processed"
echo "***** ./plcs -p -n3 -m2 beautiful .beautiful dir_1<null ****"
`./plcs -p -n2 -m2 beautiful .beautiful dir_1<null >output.log 2>err.log`
echo "OUPUT:"
cat output.log
echo
echo "ERROR MESSAGE:"
cat err.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log



echo
echo
echo "***** should print 10 lines with realpath, 1 error message and return status 0. File name starting with . is processed"
echo "***** ./plcs -ap -n2 -m2 beautiful .beautiful dir_1<null ****"
`./plcs -ap -n2 -m2 beautiful .beautiful dir_1<null >output.log 2>err.log`
echo "OUPUT:"
cat output.log
echo
echo "ERROR MESSAGE:"
cat err.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log
echo "-----------------------------------------------------------------------------"




echo
echo
echo "------------------------ Test 2.2: option -q ----------------------"
echo "***** should print 24 lines with realpath,nos, 5 error message and return status 0"
echo "***** ./plcs -p -d2 -n3 -m2 beautiful dir_1 dir_2<null ****"
`./plcs -p -n3 -d2 -m2 beautiful dir_1 dir_2<null >output.log 2>err.log`
echo "OUPUT:"
cat output.log
echo
echo "ERROR MESSAGE:"
cat err.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log



echo
echo
echo "***** should print 24 lines with realpath,nos, and return status 0"
echo "***** ./plcs -pq -d2 -n3 -m2 beautiful dir_1 dir_2<null ****"
`./plcs -pq -n3 -d2 -m2 beautiful dir_1 dir_2<null >output.log 2>err.log`
echo "OUPUT:"
cat output.log
echo
echo "ERROR MESSAGE:"
cat err.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log
echo "-----------------------------------------------------------------"




echo
echo
echo "-------------------------- Test 2.3: option -f ---------------------"
echo "***** should print 26 lines with realpath and nos and return status 0. Do not follow symbolic link, so there are no error messages for loop detecting. Symlink is command line is processed."
echo "***** ./plcs -fp -m2 -n2 beautiful dir_2<null ****"
`./plcs -fp -m2 -n2 beautiful symlink dir_2<null >output.log 2>err.log`
echo "OUPUT:"
cat output.log
echo
echo "ERROR MESSAGE:"
cat err.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log
echo "-------------------------------------------------------------------"



echo
echo
echo "-------------------------- Test 2.4 -------------------------------"
echo "***** should print 30 lines with realpath,nos,1 error message and return status 0"
echo "***** ./plcs -afqp -d2 -t10 -m2 -n2 beautiful .beautiful no-such-file symlink dir_1 dir_2<null ****"
`./plcs -afqp -d2 -t10 -m2 -n2 beautiful .beautiful no-such-file symlink dir_1 dir_2<null >output.log 2>err.log`
echo "OUPUT:"
cat output.log
echo
echo "ERROR MESSAGE:"
cat err.log
echo "status $EXIT"
echo "@@@@@ expected status 0, got status $EXIT"
l1=$(wc -l output.log | awk '{print $1}')
l2=$(wc -l err.log | awk '{print $1}')
l=$((l1+l2))
echo "@@@@@ line of output $l1, line of error message $l2, total lines $l"
rm output.log err.log



cd dir_1
chmod ugo+r nopermission
cd ..
rm -R dir_1
rm -R dir_2
rm -R symlink

