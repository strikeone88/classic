@echo off
if "%1"=="" goto all

echo %1...
bp Pretty="1" FILE="%1" make.ks temp.ks >nul
ks temp.ks >nul
goto end

:all
del output\*.bmp

dir /b *.bmp >list

iter make2 list

del temp.bmp
del temp.ks
del list

:end
