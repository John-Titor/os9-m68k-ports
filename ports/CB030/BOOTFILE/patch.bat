@echo off
if not exist %1 (
        echo input file does not exist
        exit 1
) else if "%MWOS%"=="" (
        echo MWOS not set
        exit 1
) else if "%LOCAL%"=="" (
        echo LOCAL not set
        exit 1
) else (
        for /f "eol=* tokens=1,2" %%a in (%1) do (
                if "%%a"=="MWOS" echo %MWOS%/%%b
                if "%%a"=="LOCAL" echo %LOCAL%/%%b
        )
)
