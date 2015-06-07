@echo off

cd ..\src
make rmake.exe
cd ..\tests

cd libc
call rmake

cd ..\dxe
make

cd ..\libclink
make

cd ..
