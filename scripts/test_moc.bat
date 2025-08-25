@echo off
cd /d "C:\Github\WindowsDynamicLightingSync\src"
"C:\Qt\6.9.1\msvc2022_64\bin\moc.exe" -DWINDOWSDYNAMICLIGHTINGSYNC_LIBRARY -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB --include "C:/Github/WindowsDynamicLightingSync/src/_intermediate_release/.moc/moc_predefs.h" -I"../dependencies/OpenRGBSamplePlugin/OpenRGB" -I"C:/Qt/6.9.1/msvc2022_64/include" -I"C:/Qt/6.9.1/msvc2022_64/include/QtCore" -I"C:/Qt/6.9.1/msvc2022_64/include/QtWidgets" -I"C:/Qt/6.9.1/msvc2022_64/include/QtGui" WindowsDynamicLightingSync.h -o test_moc3.cpp
echo MOC Exit Code: %ERRORLEVEL%
pause