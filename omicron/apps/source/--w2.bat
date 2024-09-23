@echo off

cc /c /I..\include w2.c sh.c glink.c trig.c scanner.c star.c text.c
if errorlevel 1 goto end
cc /c /I..\include sshot.c hboom.c squake.c item.c audio.c w2mixer.c
if errorlevel 1 goto end
cc /c /I..\include cyan.c magenta.c mmagenta.c ship.c trail.c exp.c
if errorlevel 1 goto end

obj2vo w2.obj
obj2vo sh.obj
obj2vo glink.obj
obj2vo trig.obj
obj2vo scanner
obj2vo star
obj2vo text
obj2vo ship
obj2vo shield
obj2vo sshot
obj2vo hboom
obj2vo squake
obj2vo trail
obj2vo item

obj2vo cyan
obj2vo magenta
obj2vo mmagenta
obj2vo exp

obj2vo audio
obj2vo w2mixer

vl -F -s -fcfx -o../out.bin @w2.lst

del *.obj
del *.vo

:end
