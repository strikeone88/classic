@echo off
if "%1"=="" goto normal
pcc -p-d -pyys -p-i -pyyi -p-r -s-tagscan -p-s -s-a e-s-h -p-e asm$.sx
goto end

:normal
pcc -s-tagscan -p-s -s-a -s-h -p-e asm$.sx
:end
