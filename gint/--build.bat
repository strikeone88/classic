@echo off
tcc -I. -I\kit -ml -c gint.c
del *.obj