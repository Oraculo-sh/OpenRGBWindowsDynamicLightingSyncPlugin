@echo off
cd /d "C:\Github\WindowsDynamicLightingSync\src"

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Compile WindowsDynamicLightingSync.cpp with Qt 5.15.0
echo Compiling WindowsDynamicLightingSync.cpp with Qt 5.15.0...
cl -c -nologo -O2 -MD -std:c++17 -EHsc -DWINDOWSDYNAMICLIGHTINGSYNC_LIBRARY -DVERSION_STRING="\"1.0\"" -DGIT_COMMIT_ID="\"\"" -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I. -I"../dependencies/OpenRGBSamplePlugin/OpenRGB" -I"../dependencies/OpenRGBSamplePlugin/OpenRGB/i2c_smbus" -I"C:/Qt/5.15.0/msvc2019_64/include" -I"C:/Qt/5.15.0/msvc2019_64/include/QtCore" -I"C:/Qt/5.15.0/msvc2019_64/include/QtWidgets" -I"C:/Qt/5.15.0/msvc2019_64/include/QtGui" -Fo"_intermediate_release/.obj/" WindowsDynamicLightingSync.cpp

echo Compiler Exit Code: %ERRORLEVEL%
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Compilation successful!
pause