@echo off
dir /b *.bmp >list
..\..\..\nlag list ../../../../data/line.nla 0
