@echo off
pcc -p-s -p-e ks.sx
if errorlevel 1 goto end

tasm /m3 /ml xmsh

tcc -DCONST -I\kit -I\pcc -w-par -mh -Z -k- -G -O -d @file.lst

:end
