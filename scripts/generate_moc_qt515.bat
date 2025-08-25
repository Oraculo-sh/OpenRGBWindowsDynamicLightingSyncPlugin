@echo off
echo Generating MOC file with Qt 5.15.0...

"C:\Qt\5.15.0\msvc2019_64\bin\moc.exe" ^
-DUNICODE ^
-D_UNICODE ^
-DWIN32 ^
-D_ENABLE_EXTENDED_ALIGNED_STORAGE ^
-DWIN64 ^
-DWINDOWSDYNAMICLIGHTINGSYNC_LIBRARY ^
-DVERSION_STRING="1.0" ^
-DBUILDDATE_STRING="25/08/2025" ^
-DGIT_COMMIT_ID="" ^
-DGIT_COMMIT_DATE="" ^
-DGIT_BRANCH="" ^
-DLATEST_BUILD_URL="" ^
-D_MBCS ^
-DWIN32 ^
-D_CRT_SECURE_NO_WARNINGS ^
-D_WINSOCK_DEPRECATED_NO_WARNINGS ^
-DWIN32_LEAN_AND_MEAN ^
-DNDEBUG ^
-DQT_NO_DEBUG ^
-DQT_PLUGIN ^
-DQT_WIDGETS_LIB ^
-DQT_GUI_LIB ^
-DQT_CORE_LIB ^
-D_WINDLL ^
-I. ^
-I"../dependencies/OpenRGBSamplePlugin/OpenRGB" ^
-I"../dependencies/OpenRGBSamplePlugin/OpenRGB/i2c_smbus" ^
-I"../dependencies/OpenRGBSamplePlugin/OpenRGB/RGBController" ^
-I"../dependencies/OpenRGBSamplePlugin/OpenRGB/net_port" ^
-I"../dependencies/OpenRGBSamplePlugin/OpenRGB/dependencies/json" ^
-I"C:/Qt/5.15.0/msvc2019_64/include" ^
-I"C:/Qt/5.15.0/msvc2019_64/include/QtWidgets" ^
-I"C:/Qt/5.15.0/msvc2019_64/include/QtGui" ^
-I"C:/Qt/5.15.0/msvc2019_64/include/QtANGLE" ^
-I"C:/Qt/5.15.0/msvc2019_64/include/QtCore" ^
-I"C:/Qt/5.15.0/msvc2019_64/mkspecs/win32-msvc" ^
WindowsDynamicLightingSync.h ^
-o _intermediate_release\.moc\moc_WindowsDynamicLightingSync.cpp

echo MOC generation completed with exit code: %ERRORLEVEL%
if %ERRORLEVEL% neq 0 (
    echo MOC generation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo MOC file generated successfully!