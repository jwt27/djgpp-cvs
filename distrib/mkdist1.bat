@echo off

becho Making %1... 
cd ..

echo %1 %2 %3 %4 %5 %6 %7 %8 %9 > manifest\%1.ver

becho find... 
find @distrib/p/%1/files -type f -print > manifest\%1.mft

if not exist distrib\p\%1\skips goto noskip
becho skip... 
sed -f distrib/p/%1/skips manifest/%1.mft > manifest\%1.tmp
mv manifest/%1.tmp manifest/%1.mft
:noskip

echo manifest/%1.mft>>manifest\%1.mft
echo manifest/%1.ver>>manifest\%1.mft

if not exist distrib\p\%1\nobins goto dobins
becho nobins... 
cp manifest/%1.mft manifest/%1.tmp
distrib\nobins @distrib/p/%1/nobins < manifest\%1.tmp > manifest\%1.tm2
mv manifest/%1.tm2 manifest/%1.mft
:dobins

becho sort... 
sort < manifest\%1.mft > manifest\%1.tmp
cp manifest/%1.tmp manifest/%1.mft

becho zip... 
if exist zips\%1.zip del zips\%1.zip
zip -qq -9 -@ zips/%1.zip < manifest\%1.tmp
rm manifest/%1.tmp

echo done.

ls -l zips/%1.zip

cd distrib

echo.
