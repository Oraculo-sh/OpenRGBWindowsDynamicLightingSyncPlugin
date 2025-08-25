@echo off
cd /d "C:\Github\WindowsDynamicLightingSync\src"

REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Compile WindowsDynamicLightingSync.cpp
echo Compiling WindowsDynamicLightingSync.cpp...
cl -c -nologo -Zc:wchar_t -FS -Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -permissive- -Zc:__cplusplus -Zc:externConstexpr -O2 -MD -std:c++17 -utf-8 -W3 -w34100 -w34189 -w44456 -w44457 -w44458 -wd4577 -wd4467 -EHsc -DUNICODE -D_UNICODE -DWIN32 -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DWINDOWSDYNAMICLIGHTINGSYNC_LIBRARY -DVERSION_STRING=\"1.0+ (git)\" -DBUILDDATE_STRING=\"25/08/2025\" -DGIT_COMMIT_ID=\"\" -DGIT_COMMIT_DATE=\"\" -DGIT_BRANCH=\"\" -DLATEST_BUILD_URL=\"\" -D_MBCS -DWIN32 -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -DWIN32_LEAN_AND_MEAN -DNDEBUG -DQT_NO_DEBUG -DQT_PLUGIN -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -D_WINDLL -I. -I"../dependencies/OpenRGBSamplePlugin/OpenRGB" -I"../dependencies/OpenRGBSamplePlugin/OpenRGB/i2c_smbus" -I"../dependencies/OpenRGBSamplePlugin/OpenRGB/RGBController" -I"../dependencies/OpenRGBSamplePlugin/OpenRGB/net_port" -I"../dependencies/OpenRGBSamplePlugin/OpenRGB/dependencies/json" -I"C:/Qt/6.9.1/msvc2022_64/include" -I"C:/Qt/6.9.1/msvc2022_64/include/QtWidgets" -I"C:/Qt/6.9.1/msvc2022_64/include/QtGui" -I"C:/Qt/6.9.1/msvc2022_64/include/QtCore" -I"_intermediate_release/.moc" -I"C:/Qt/6.9.1/msvc2022_64/mkspecs/win32-msvc" -Fo"_intermediate_release/.obj/" WindowsDynamicLightingSync.cpp

echo Compiler Exit Code: %ERRORLEVEL%
if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Compilation successful!
pause