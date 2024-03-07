@echo off
setlocal

call powershell -Command "& .\dotnet-install.ps1 -NoPath -InstallDir .dotnet -Version 8.0.101"
call powershell -File .\setup.ps1
if not %ERRORLEVEL% == 0 exit %ERRORLEVEL%
.dotnet\dotnet.exe restore