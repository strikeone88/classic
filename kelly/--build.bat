@echo off
if "%1"=="asm" goto buildAsm
if "%1"=="c" goto buildC

if exist k3.lib del k3.lib

rem **********************************************
call --build asm kgfx
call --build c k3d
rem **********************************************

goto end

:buildC
cc386 /o%2 /c -IINCLUDE source/c/%2
xlib k3 +%2.obj
if exist k3.bak del k3.bak
del %2.obj
goto end

:buildAsm
oz source/asm/%2
vo2obj %2
del %2.vo
xlib k3 +%2.obj
if exist k3.bak del k3.bak
del %2.obj

:end
