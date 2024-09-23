@echo off
tcc -DCONST -D__GLINK_DEBUGGING -w-par -ml -k- -Z -G -O -d -I\kit -I\pcc -I. @file.lst
del *.obj
:end
