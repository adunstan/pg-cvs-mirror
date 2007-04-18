@echo off
REM $PostgreSQL: pgsql/src/tools/msvc/build.bat,v 1.7 2007/03/17 14:01:01 mha Exp $

SETLOCAL
SET STARTDIR=%CD%
SET CONFIG=
if exist src\tools\msvc\buildenv.bat call src\tools\msvc\buildenv.bat
if exist buildenv.bat call buildenv.bat

perl mkvcbuild.pl
if errorlevel 1 goto :eof

if exist ..\msvc if exist ..\..\..\src cd ..\..\..
set CFG=
if "%1" == "DEBUG" (
 set CONFIG=Debug
 set CFG=1
)
if "%1" == "RELEASE" (
 set CONFIG=Release
 set CFG=1
)
if "%CONFIG%" == "" set CONFIG=Release

if "%CFG%" == "1" shift

echo Building %CONFIG%

if "%1" == "" msbuild pgsql.sln /verbosity:detailed /p:Configuration=%CONFIG%
if not "%1" == "" vcbuild %1.vcproj %CONFIG%
SET E=%ERRORLEVEL%

cd %STARTDIR%

exit /b %E%
