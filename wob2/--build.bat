@echo off
tcc -D__GLINK_DEBUGGING -w-par -ml -Ilib -Isprites -I. -I\kit @file.lst
del *.obj