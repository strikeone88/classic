@echo off
if exist *.obj del *.obj
if exist log del log

if "%1"=="log" goto log
if not "%1"=="" goto single

for %%x in (*.c) do ..\bin\cc /w-ret /w-pro /w-spc /Iinclude /c %%x

..\bin\tasm /m3 /ml start

if exist ..\obj\*.obj del ..\obj\*.obj
copy *.obj ..\obj >nul

goto end

:single
..\bin\cc /w-ret /w-pro /w-spc /Iinclude /S %1
goto end

:log
for %%x in (*.c) do ..\bin\cc /w-ret /w-pro /w-spc /Iinclude /c %%x >>log

:end
if exist *.obj del *.obj
