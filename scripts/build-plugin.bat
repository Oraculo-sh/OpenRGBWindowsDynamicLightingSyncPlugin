@echo off
REM -----------------------------------------------------------------------
REM Windows Dynamic Lighting Sync Plugin Build Script
REM -----------------------------------------------------------------------

setlocal enabledelayedexpansion

echo Building Windows Dynamic Lighting Sync Plugin...
echo.

REM -----------------------------------------------------------------------
REM Set up environment variables
REM -----------------------------------------------------------------------
set PROJECT_DIR=%~dp0..
set SRC_DIR=%PROJECT_DIR%\src
set RELEASE_DIR=%PROJECT_DIR%\release
set PROJECT_FILE=WindowsDynamicLightingSync.pro

REM -----------------------------------------------------------------------
REM Configure Visual Studio Environment
REM -----------------------------------------------------------------------
echo Configuring Visual Studio environment...

REM Try different Visual Studio versions and paths
set VS_CONFIGURED=0

REM Visual Studio 2022
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    set VS_CONFIGURED=1
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    set VS_CONFIGURED=1
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    set VS_CONFIGURED=1
)

REM Visual Studio 2019
if !VS_CONFIGURED! equ 0 (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
        set VS_CONFIGURED=1
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
        set VS_CONFIGURED=1
    )
)

if !VS_CONFIGURED! equ 0 (
    echo WARNING: Visual Studio environment not configured automatically.
    echo Please ensure Visual Studio Build Tools are installed.
)

REM -----------------------------------------------------------------------
REM Configure Qt Environment
REM -----------------------------------------------------------------------
echo Configuring Qt environment...

REM Check if Qt is already in PATH
qmake --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Qt not found in PATH, searching for Qt installation...
    
    REM Try to find Qt installation
    set QT_FOUND=0
    
    REM Check common Qt installation paths
    for %%V in (5.15.0 5.15.2 6.8.3 6.9.1) do (
        if exist "C:\Qt\%%V\msvc2019_64\bin\qmake.exe" (
            set QTDIR=C:\Qt\%%V\msvc2019_64
            set PATH=!QTDIR!\bin;!PATH!
            set QT_FOUND=1
            echo Found Qt %%V at !QTDIR!
            goto :qt_configured
        )
        if exist "C:\Qt\%%V\msvc2022_64\bin\qmake.exe" (
            set QTDIR=C:\Qt\%%V\msvc2022_64
            set PATH=!QTDIR!\bin;!PATH!
            set QT_FOUND=1
            echo Found Qt %%V at !QTDIR!
            goto :qt_configured
        )
    )
    
    :qt_configured
    if !QT_FOUND! equ 0 (
        echo ERROR: Qt not found. Please install Qt 5.15 or later.
        echo Expected locations:
        echo   C:\Qt\5.15.0\msvc2019_64\
        echo   C:\Qt\5.15.2\msvc2019_64\
        echo   C:\Qt\6.x.x\msvc2022_64\
        pause
        exit /b 1
    )
) else (
    echo Qt already configured in PATH.
)

REM -----------------------------------------------------------------------
REM Verify build environment
REM -----------------------------------------------------------------------
echo Verifying build environment...

qmake --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: qmake still not found after configuration.
    pause
    exit /b 1
)

echo Build environment configured successfully.
echo.

REM -----------------------------------------------------------------------
REM Navigate to source directory
REM -----------------------------------------------------------------------
cd /d "%SRC_DIR%"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Could not navigate to source directory: %SRC_DIR%
    pause
    exit /b 1
)

echo Current directory: %CD%
echo Project file: %PROJECT_FILE%
echo.

REM -----------------------------------------------------------------------
REM Configure build files with qmake
REM -----------------------------------------------------------------------
echo Configuring project with qmake...
qmake "%PROJECT_FILE%"
if %ERRORLEVEL% neq 0 (
    echo ERROR: qmake configuration failed!
    pause
    exit /b 1
)

echo qmake configuration completed successfully.
echo.

REM -----------------------------------------------------------------------
REM Build the project
REM -----------------------------------------------------------------------
echo Building project...

REM Use nmake for build (Visual Studio)
echo Using nmake for build...
nmake
set BUILD_RESULT=%ERRORLEVEL%

if !BUILD_RESULT! neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo Build completed successfully.
echo.

REM -----------------------------------------------------------------------
REM Copy output to release directory
REM -----------------------------------------------------------------------
echo Copying output to release directory...

REM Create release directory if it doesn't exist
if not exist "%RELEASE_DIR%" (
    mkdir "%RELEASE_DIR%"
    echo Created release directory: %RELEASE_DIR%
)

REM Find and copy the DLL file
set DLL_FOUND=0
for %%D in (release debug) do (
    if exist "%%D\WindowsDynamicLightingSync.dll" (
        copy "%%D\WindowsDynamicLightingSync.dll" "%RELEASE_DIR%\" >nul
        if !ERRORLEVEL! equ 0 (
            echo Successfully copied WindowsDynamicLightingSync.dll to release directory.
            set DLL_FOUND=1
            goto :dll_copied
        )
    )
)

REM Check current directory as well
if !DLL_FOUND! equ 0 (
    if exist "WindowsDynamicLightingSync.dll" (
        copy "WindowsDynamicLightingSync.dll" "%RELEASE_DIR%\" >nul
        if !ERRORLEVEL! equ 0 (
            echo Successfully copied WindowsDynamicLightingSync.dll to release directory.
            set DLL_FOUND=1
        )
    )
)

:dll_copied
if !DLL_FOUND! equ 0 (
    echo WARNING: WindowsDynamicLightingSync.dll not found in expected locations.
    echo Please check the build output manually.
)

echo.
echo -----------------------------------------------------------------------
echo Build process completed!
echo -----------------------------------------------------------------------
echo Plugin DLL location: %RELEASE_DIR%\WindowsDynamicLightingSync.dll
echo.
echo To install the plugin:
echo 1. Copy the DLL to your OpenRGB plugins directory
echo 2. Restart OpenRGB
echo 3. The plugin should appear in the plugins list
echo -----------------------------------------------------------------------

pause