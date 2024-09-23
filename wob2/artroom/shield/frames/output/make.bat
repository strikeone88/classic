@echo off
dir /b *.bmp >list
..\..\..\nlag list ../../../../data/shield.nla 2
