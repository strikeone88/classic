@echo off
if "%1"=="" goto end
if "%2"=="" goto end

bp InFile="%1" wordtbl.bp %2

:end
