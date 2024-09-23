@echo off
..\bin\tasm /m3 /ml gboot.asm
..\bin\linx -x -f bin -o ..\boot gboot
del gboot.obj
