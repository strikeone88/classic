@echo off
rem tcc -D__AURORA_ONDISK -w-par -ml -k- -Z -G -O -d -I\kit -I\pcc @file.lst
bcc32 -D__AURORA_ONDISK -w- -k- -Z -G -O -d -I\kit -I\pcc @file.lst
del *.obj
del *.tds
:end
