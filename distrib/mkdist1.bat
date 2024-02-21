@echo off

echo Making %1...
cd ..

echo %1 %2 %3 %4 %5 %6 %7 %8 %9 > manifest\%1.ver

echo find...
find @distrib/p/%1/files -type f -print > manifest\%1.mft

if not exist distrib\p\%1\skips goto noskip
echo skip...
sed -f distrib/p/%1/skips manifest/%1.mft > manifest\%1.tmp
mv manifest/%1.tmp manifest/%1.mft
:noskip

echo manifest/%1.mft>>manifest\%1.mft
echo manifest/%1.ver>>manifest\%1.mft

if not exist distrib\p\%1\%1.dsm goto nodsm
echo dsm...
cp distrib\p\%1\%1.dsm manifest/%1.dsm
echo manifest/%1.dsm>>manifest\%1.mft
:nodsm

if not exist distrib\p\%1\nobins goto dobins
echo nobins...
cp manifest/%1.mft manifest/%1.tmp
if not exist distrib\nobins.exe gcc -s -O2 distrib/nobins.c -o distrib/nobins.exe
distrib\nobins @distrib/p/%1/nobins < manifest\%1.tmp > manifest\%1.tm2
mv manifest/%1.tm2 manifest/%1.mft
:dobins

echo sort...
sort < manifest\%1.mft > manifest\%1.tmp
cp manifest/%1.tmp manifest/%1.mft

echo zip...
if exist zips\%1.zip del zips\%1.zip
zip -qq -9 -@ zips/%1.zip < manifest\%1.tmp
rm manifest/%1.tmp

echo done.

ls -l zips/%1.zip

cd distrib

echo.
