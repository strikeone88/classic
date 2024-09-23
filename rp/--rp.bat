@echo off
bcc32 -w-8012 -w-8057 -Ideps -G -d -Z -k- -O rp.c deps\angela.c deps\eve.c
del *.tds
