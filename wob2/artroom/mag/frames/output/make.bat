@echo off
dir /b *.bmp >list
..\..\..\nlag list ../../../../data/mag.nla 0
