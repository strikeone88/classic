@echo off
if exist *.obj goto buildVo

:resume
vl -F -p -m kernel.map -o..\kernel.bin -s -G -B 180000 @file.lst

goto end

:buildVo
for %%x in (*.obj) do obj2vo %%x
del *.obj
goto resume

:end
