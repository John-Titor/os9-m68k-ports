@echo off
::
:: Create distribution/installation files for OSK
::
setlocal

:: locate the SDK
if "%MWOS%"=="" ( 
set MWOS=M:\MWOS
)

:: add DOS utilities to the path
set PATH=%CD%\..\dos;%PATH%

:: clean out the destination directory
set DEST=%CD%\archives
rmdir /q /s %DEST%
mkdir %DEST%

:: choose a location for the staging directory
if "%TEMP%"=="" (
set TEMP=.
)
set STAGE=%TEMP%\dist.tmp

:: (re) initialize the staging area
rmdir /q /s %STAGE%
mkdir %STAGE%
mkdir %STAGE%\CMDS
mkdir %STAGE%\BOOTOBJS
mkdir %STAGE%\SYS

:: populate the staging area
for /f "tokens=1" %%a in (filesets\cmds) do (
	copy %MWOS%\OS9\68000\CMDS\%%a %STAGE%\CMDS
)
for /f %%a in ('dir /b ..\CMDS') do (
	copy ..\CMDS\%%a %STAGE%\CMDS
)
for /f %%a in ('dir /b ..\apps\bin') do (
	copy ..\apps\bin\%%a %STAGE%\CMDS
)
for /f "tokens=1" %%a in (filesets\bootobjs) do (
	copy %MWOS%\OS9\%%a %STAGE%\BOOTOBJS
)
for /f %%a in ('dir /b .\SYS') do (
	copy .\SYS\%%a %STAGE%\SYS
)
copy %MWOS%\OS9\SRC\SYS\errmsg %STAGE%\SYS
copy %MWOS%\OS9\SRC\SYS\moded.fields %STAGE%\SYS
copy %MWOS%\OS9\SRC\SYS\termcap %STAGE%\SYS
copy %MWOS%\OS9\SRC\SYS\umacs.hlp %STAGE%\SYS

:: build the distribution archives
7z a -r -tZIP %DEST%\osk_cmds.zip %STAGE%\CMDS\*
7z a -r -tZIP %DEST%\osk_bootobjs.zip %STAGE%\BOOTOBJS\*
7z a -r -tZIP %DEST%\osk_sys.zip %STAGE%\SYS\*
