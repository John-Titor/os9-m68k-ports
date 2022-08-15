@echo off
::
:: Wrap os9make with the tools in PATH
::
:: OS9 SDK install path defaults to M:\MWOS
:: 
if "%MWOS%"=="" ( 
set MWOS=M:\MWOS
)
set PATH=%MWOS%\DOS\BIN;%PATH%
os9make MWOS=%MWOS% %*
