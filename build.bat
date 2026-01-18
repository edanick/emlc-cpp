@echo off
setlocal
cd /d "%~dp0"

set VCVARS="D:\Programs\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"
set CONFIG=Debug

:parse_args
if "%~1"=="" goto build
if /i "%~1"=="--release" set CONFIG=Release
if /i "%~1"=="--debug" set CONFIG=Debug
shift
goto parse_args

:build
if exist %VCVARS% (
    call %VCVARS%
    MSBuild emlc\emlc.vcxproj /p:Configuration=%CONFIG% /p:Platform=x64
    
    if errorlevel 1 (
        echo Build failed
        exit /b 1
    )

    REM Copy output to current directory for convenience
    if exist "emlc\x64\%CONFIG%\emlc.exe" (
        copy /Y "emlc\x64\%CONFIG%\emlc.exe" "emlc.exe"
        echo Build successful: emlc.exe [%CONFIG%]
    ) else (
        echo Error: Build succeeded but output file not found at expected path: emlc\x64\%CONFIG%\emlc.exe
    )
) else (
    echo Error: Visual Studio not found at expected path
    echo Please update VCVARS path in this script
)
