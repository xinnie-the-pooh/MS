@echo off
build -cZDbe
if not exist init\console\obj\i386\xboxkrnl.exe goto nofile
rem call makekrnl.bat
goto pack

:nofile
echo kernel not found!
goto exit

:pack
copy \xbox\private\ntos\init\console\obj\i386\xboxkrnl.exe \xbox\bios\xboxkrnl.img
BIOSpack -t Multi -i \xbox\bios\ -o \xbox\bios\xboxrom.bin


:exit