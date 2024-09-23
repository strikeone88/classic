@echo off
if "%1"=="" goto all

echo %1...
bp FILE="%1" make.ks temp.ks >nul
ks temp.ks >nul
goto end

:all
del output\*.bmp

dir /b *.bmp >list

iter make list

del temp.ks
del list

:end
