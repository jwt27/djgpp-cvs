@echo off

make
if errorlevel 1 goto exit

echo ==== libc
cd libc
..\rmake
if errorlevel 1 goto exit

echo ==== libm
cd ..\libm
..\rmake
if errorlevel 1 goto exit

echo ==== debug
cd ..\debug
..\rmake
if errorlevel 1 goto exit
..\rmake e
if errorlevel 1 goto exit

echo ==== dxe
cd ..\dxe
make
if errorlevel 1 goto exit

echo ==== libemu
cd ..\libemu
make
if errorlevel 1 goto exit

echo ==== mkdoc
cd ..\mkdoc
make
if errorlevel 1 goto exit

echo ==== doc
cd ..\libc
make doc
if errorlevel 1 goto exit

echo ==== stub
cd ..\stub
make
if errorlevel 1 goto exit

echo ==== utils
cd ..\utils
make
if errorlevel 1 goto exit

cd ..
:exit
