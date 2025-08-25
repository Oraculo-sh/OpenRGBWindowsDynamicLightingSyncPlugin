@echo off
cd /d "C:\Github\WindowsDynamicLightingSync\src"

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Try to compile with minimal Qt includes to isolate the problem
echo Compiling with minimal includes...
cl -c -nologo -O2 -MD -std:c++17 -Zc:__cplusplus -EHsc -DWINDOWSDYNAMICLIGHTINGSYNC_LIBRARY -DVERSION_STRING="\"1.0\"" -DGIT_COMMIT_ID="\"\"" -I. -I"../dependencies/OpenRGBSamplePlugin/OpenRGB" -I"../dependencies/OpenRGBSamplePlugin/OpenRGB/i2c_smbus" -Fo"_intermediate_release/.obj/" WindowsDynamicLightingSync.cpp

echo Compiler Exit Code: %ERRORLEVEL%
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Compilation successful!
pause