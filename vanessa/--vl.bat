@echo off
tcc -D__AURORA_ONDISK -DNO_BIG_ENDIAN -w-par -ml -k- -Z -G -O -d -I\kit @file.lst
del *.obj
:end
