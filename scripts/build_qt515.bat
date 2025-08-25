@echo off
cd /d "C:\Github\WindowsDynamicLightingSync\src"

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Set Qt 5.15.0 environment
set QTDIR=C:\Qt\5.15.0\msvc2019_64
set PATH=%QTDIR%\bin;%PATH%

REM Clean previous build files
echo Cleaning previous build files...
if exist Makefile del Makefile
if exist Makefile.Debug del Makefile.Debug
if exist Makefile.Release del Makefile.Release
if exist "_intermediate_debug" rmdir /s /q "_intermediate_debug"
if exist "_intermediate_release" rmdir /s /q "_intermediate_release"

REM Generate Makefiles with qmake
echo Generating Makefiles with Qt 5.15.0...
qmake WindowsDynamicLightingSync.pro -spec win32-msvc

if %ERRORLEVEL% neq 0 (
    echo qmake failed!
    pause
    exit /b %ERRORLEVEL%
)

REM Build the project
echo Building project...
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