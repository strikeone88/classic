@echo off
if not exist kernel.img goto build

:resume
bin\wrimg kernel.img 0 A
goto end

:build
call --image
if exist kernel.img goto resume

:end
