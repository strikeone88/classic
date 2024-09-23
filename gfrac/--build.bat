@echo off
tcc -I. -I\kit -ml -c gfrac.c
del *.obj
