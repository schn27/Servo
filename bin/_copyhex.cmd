@echo off
set "curpath=%cd%"
cd "%~dp0"
copy .\Obj\Servo.hex > nul
cd %curpath%
