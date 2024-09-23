@echo off
tcc -DCONST -I\kit -I\pcc -w-par -mh -Z -k- -G -O -d @file.lst
:end
