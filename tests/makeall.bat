@echo off

cd libc
call rmake

cd ..\dxe
make

cd ..\libclink
make

cd ..
