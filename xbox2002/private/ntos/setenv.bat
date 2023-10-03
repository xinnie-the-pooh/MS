@echo off
if "%1"=="" goto error

:good
echo Setting up environment variables

SET BASEDIR=%1\XBOX
SET NTMAKEENV=%1\xbox\public\oak\bin
SET _NTTREE=%1\XBOX
path=%path%;C:\WINDDK\3790\bin\x86;%1\xbox\tools;%1\xbox\private\idw;C:\Program Files\Microsoft Visual Studio .NET\Vc7\bin

echo Done!
goto done

:error
echo Usage: setenv (driveletter):
echo Like "setenv F:"

:done