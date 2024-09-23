@echo off
..\..\bin\cc /I..\include /c lib.c
..\..\bin\tasm /m3 /ml start
oz extra
oz pcihw

obj2vo start
obj2vo lib

del *.obj

if exist slib.lib del slib.lib
vl -Lslib lib.vo extra.vo pcihw.vo

:end