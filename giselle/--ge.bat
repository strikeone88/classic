@echo off
tcc -v -y -ml -w-par -I. -I\kit -k- -Z -G -O -d @file.lst
del *.obj
