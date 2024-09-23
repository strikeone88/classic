@echo off
if exist obj\*.vo del obj\*.vo
if exist *.bin del *.bin
if exist *.img del *.img

cd apps
call --clean
cd ..
