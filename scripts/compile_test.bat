@echo off
echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

echo Compiling WindowsDynamicLightingSync.cpp...
cd /d "C:\Github\WindowsDynamicLightingSync\src"
cl /c /nologo WindowsDynamicLightingSync.cpp /I..\dependencies\OpenRGBSamplePlugin\OpenRGB /IC:\Qt\6.9.1\msvc2022_64\include /IC:\Qt\6.9.1\msvc2022_64\include\QtCore /IC:\Qt\6.9.1\msvc2022_64\include\QtWidgets /IC:\Qt\6.9.1\msvc2022_64\include\QtGui /DWIN32 /D_WINDOWS /DQT_WIDGETS_LIB /DQT_GUI_LIB /DQT_CORE_LIB

echo Compilation test complete!
pause