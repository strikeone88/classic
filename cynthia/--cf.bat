@echo off
oz main.asm
oz core.asm
vl -S512 -G main.vo
vl -S65536 -G -B100000 core.vo
copy /b main.bin+core.bin cf.bin
del *.vo
del main.bin
del core.bin
:end