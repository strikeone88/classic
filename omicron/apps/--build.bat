@echo off
if "%1"=="all" goto all
if "%1"=="" goto end
if "%1"=="#" goto end
if "%1"=="!" goto list
if "%1"=="@" goto custom

echo *******************
echo *** BUILDING %1 ***
echo *******************

..\bin\cc /oout /c /Isource /Iinclude source/%1.c
obj2vo out.obj
del out.obj

..\bin\vl -fcfx -oout.bin lib/start out lib/slib.lib
del out.vo

copy /b apps.bin+out.bin temp
if exist apps.bin del apps.bin
ren temp apps.bin
del out.bin

goto end

:list
shift

echo *******************
echo *** BUILDING %1 %2 ***
echo *******************

..\bin\cc /oout1 /c /Isource /Iinclude source/%1.c
..\bin\cc /oout2 /c /Isource /Iinclude source/%2.c

..\bin\obj2vo out1.obj
..\bin\obj2vo out2.obj
del out1.obj
del out2.obj

..\bin\vl -fcfx -oout.bin lib/start out1 out2 lib/slib.lib
del out1.vo
del out2.vo

copy /b apps.bin+out.bin temp
if exist apps.bin del apps.bin
ren temp apps.bin
del out.bin

goto end

:custom
cd source
call %2
cd ..

copy /b apps.bin+out.bin temp
if exist apps.bin del apps.bin
ren temp apps.bin
del out.bin

goto end

:all

if not exist lib\start.vo goto buildlib

:resume

if exist apps.bin del apps.bin
iter --build file-lst.txt

if exist ..\apps.bin del ..\apps.bin
copy apps.bin ..
del apps.bin

goto end

:buildlib
cd lib
call --build
cd ..

goto resume

:end
