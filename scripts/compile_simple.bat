@echo off
cd /d "C:\Github\WindowsDynamicLightingSync\src"

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Compile WindowsDynamicLightingSync.cpp with basic flags
echo Compiling WindowsDynamicLightingSync.cpp...
cl -c -nologo -O2 -MD -std:c++17 -Zc:__cplusplus -EHsc -DWINDOWSDYNAMICLIGHTINGSYNC_LIBRARY -DVERSION_STRING="\"1.0\"" -DGIT_COMMIT_ID="\"\"" -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I. -I"../dependencies/OpenRGBSamplePlugin/OpenRGB" -I"../dependencies/OpenRGBSamplePlugin/OpenRGB/i2c_smbus" -I"C:/Qt/6.9.1/msvc2022_64/include" -I"C:/Qt/6.9.1/msvc2022_64/include/QtCore" -I"C:/Qt/6.9.1/msvc2022_64/include/QtWidgets" -I"C:/Qt/6.9.1/msvc2022_64/include/QtGui" -Fo"_intermediate_release/.obj/" WindowsDynamicLightingSync.cpp

echo Compiler Exit Code: %ERRORLEVEL%
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Compilation successful!
pause