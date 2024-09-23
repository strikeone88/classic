@echo off
if "%1"=="" goto all

cd %1
cd frames
del *.bmp
cd output
del *.bmp
cd ..
cd ..
cd ..

goto end

:all
if exist *.obj del *.obj

dir /a:d /b >list
iter --clean list

del list

:end
