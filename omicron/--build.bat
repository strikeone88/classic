@echo off
cd boot
call --build
cd ..

cd source
call --build
cd ..

cd obj
call --build
cd ..

call --image