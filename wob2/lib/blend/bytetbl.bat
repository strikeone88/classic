@echo off
if "%1"=="" goto end
if "%2"=="" goto end

bp Name=%1 InFile="%1" bytetbl.bp %2

:end
