@echo off
cd /d "C:\Github\WindowsDynamicLightingSync\src"

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Set Qt 5.15.0 environment
set QTDIR=C:\Qt\5.15.0\msvc2019_64
set PATH=%QTDIR%\bin;%PATH%

REM Continue build
echo Continuing build...
nmake release

echo Build Exit Code: %ERRORLEVEL%
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Build successful!
dir *.dll
pause