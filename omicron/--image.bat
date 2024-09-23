@echo off
if exist kernel.img del kernel.img

if not exist kernel.bin goto build
if not exist boot.bin goto build

if not exist apps.bin goto build2

:resume
if not exist cf.bin goto build3

:resume2
copy /b boot.bin+cf.bin+kernel.bin+apps.bin kernel.img
goto end

:build
call --build
goto end

:build3
cd cynthia
call --cf
cd ..
goto resume2

:build2
cd apps
call --build all
cd ..
if exist apps.bin goto resume
echo #>apps.bin
goto resume

:end
