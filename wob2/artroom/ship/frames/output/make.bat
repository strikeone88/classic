@echo off
dir /b *.bmp >list
..\..\..\nlag list ../../../../data/ship.nla 0
