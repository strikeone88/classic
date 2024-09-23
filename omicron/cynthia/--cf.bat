@echo off
..\bin\oz main
..\bin\oz core
..\bin\vl -G main
..\bin\vl -fcfx core
copy /b main.bin+core.cfx ..\cf.bin
del main.bin
del core.cfx
del *.vo
:end