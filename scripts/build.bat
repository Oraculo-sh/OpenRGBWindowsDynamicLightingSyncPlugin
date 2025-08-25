@echo off
echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

echo Running qmake...
C:\Qt\6.9.1\msvc2022_64\bin\qmake.exe WindowsDynamicLightingSync.pro

echo Building project...
cmd /c ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\nmake.exe" -f Makefile.Release"

echo Build complete!
pause