@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
"C:\Qt\5.15.0\msvc2019_64\bin\qmake.exe" src\WindowsDynamicLightingSync.pro
echo qmake exit code: %ERRORLEVEL%
if %ERRORLEVEL% neq 0 (
    echo qmake failed!
    pause
    exit /b %ERRORLEVEL%
)
echo qmake succeeded, running nmake...
nmake